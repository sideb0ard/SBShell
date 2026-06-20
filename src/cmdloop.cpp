#include <cmdloop.h>
#include <display_queue.h>
#include <locale.h>
#include <midi_cmds.h>
#include <mixer.h>
#include <new_item_cmds.h>
#include <obliquestrategies.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synth_cmds.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <utils.h>

#include <algorithm>
#include <atomic>
#include <deque>
#include <filereader.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <tsqueue.hpp>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

extern std::unique_ptr<Mixer> global_mixr;
extern Tsqueue<std::string> eval_command_queue;
extern Tsqueue<std::string> repl_queue;
extern Tsqueue<ScheduledDisplayItem> display_queue;

#define READLINE_SAFE_MAGENTA "\001\x1b[35m\002"
#define READLINE_SAFE_RESET "\001\x1b[0m\002"
#define READLINE_SAFE_RED "\001\x1b[31m\002"
#define READLINE_SAFE_GREEN "\001\x1b[32m\002"

static const char kPromptNormal[] =
    READLINE_SAFE_MAGENTA "SB#> " READLINE_SAFE_RESET;
static const char kPromptRec[] =
    READLINE_SAFE_RED "⏺ REC> " READLINE_SAFE_RESET;
static const char kPromptLoop[] =
    READLINE_SAFE_GREEN "▶ SB#> " READLINE_SAFE_RESET;

char const *prompt = kPromptNormal;

static std::atomic<bool> terminal_resized{false};
static void handle_sigwinch(int) {
  terminal_resized.store(true, std::memory_order_relaxed);
}

static bool is_process_statement(const std::string &line) {
  size_t i = line.find_first_not_of(" \t");
  if (i == std::string::npos || line[i] != 'p') return false;
  i++;
  if (i >= line.size() || !isdigit(line[i])) return false;
  while (i < line.size() && isdigit(line[i])) i++;
  return i < line.size() && isspace(line[i]);
}

static std::string strip_line_comment(const std::string &line) {
  // Whole-line '#' comment (possibly with leading whitespace)
  size_t first = line.find_first_not_of(" \t");
  if (first != std::string::npos && line[first] == '#') return "";

  if (is_process_statement(line)) return line;

  bool in_string = false;
  char string_char = '\0';
  for (size_t i = 0; i < line.length(); i++) {
    char c = line[i];
    if (!in_string && (c == '"' || c == '\'')) {
      in_string = true;
      string_char = c;
    } else if (in_string && c == string_char &&
               (i == 0 || line[i - 1] != '\\')) {
      in_string = false;
    } else if (!in_string && c == '/' && i + 1 < line.length() &&
               line[i + 1] == '/') {
      return line.substr(0, i);
    } else if (!in_string && c == '#') {
      return line.substr(0, i);
    }
  }
  return line;
}
static bool active{true};

// Transport key handlers — only fire when the input line is empty so they
// don't interfere with normal typing.
static int transport_space(int count, int key) {
  bool transport_active =
      global_mixr->midi_loop_ || global_mixr->midi_recording;
  if (rl_end == 0 && transport_active) {
    eval_command_queue.push("midi_rec()");
    return 0;
  }
  return rl_insert(count, key);
}

static int transport_backtick(int count, int key) {
  (void)count;
  (void)key;
  bool transport_active =
      global_mixr->midi_loop_ || global_mixr->midi_recording;
  if (rl_end == 0 && transport_active) {
    eval_command_queue.push("midi_stop()");
    return 0;
  }
  return rl_insert(count, '`');
}

static int transport_escape(int count, int key) {
  (void)count;
  (void)key;
  bool transport_active =
      global_mixr->midi_loop_ || global_mixr->midi_recording;
  if (transport_active) {
    eval_command_queue.push("midi_stop()");
    return 0;
  }
  return 0;  // no-op when not in transport mode
}
// Highest row= seen across all active draw_plot/draw_bar calls.
// Cursor parks at max_plot_row+1 after Ctrl-L; scroll region starts there too.
static int max_plot_row = 0;

// Re-apply the screen-size cap before every rl_forced_update_display so
// readline never places the cursor on the status-bar row.
static void cap_and_redisplay() {
  int rows = (global_mixr->term_rows_ > 1) ? global_mixr->term_rows_ : 24;
  int cols = 80;
  rl_get_screen_size(nullptr, &cols);
  rl_set_screen_size(rows - 1, cols);
  rl_forced_update_display();
}

static int handle_ctrl_l(int count, int key) {
  (void)count;
  (void)key;
  max_plot_row = 0;
  int rows = (global_mixr->term_rows_ > 1) ? global_mixr->term_rows_ : 24;
  int cols = 80;
  rl_get_screen_size(nullptr, &cols);
  // Clear screen and restore a scroll region that keeps the status bar (last
  // row) pinned. Using 1..rows-1 means content scrolls out of row 1 into
  // the terminal's scrollback buffer while row `rows` stays fixed.
  printf("\033[2J\033[1;%dr\033[1;1H", rows - 1);
  fflush(stdout);
  rl_on_new_line();
  cap_and_redisplay();
  return 0;
}

constexpr int kFileCheckInterval = 960;  // Check once per beat (PPQN)

// ---------------------------------------------------------------------------
// Tab completion
// ---------------------------------------------------------------------------
static const char *kCompletionWords[] = {
    // Sound generators
    "drumsynth()", "subsynth()", "fmsynth()", "wavsynth()", "phasor(", "loop(",
    "sample(",
    // Language keywords
    "let ", "if (", "else", "while (", "return", "true", "false", "NULL",
    "comp ", "setup()", "run()", "break", "continue", "fn ",
    // Note / playback
    "note_on(", "note_off(", "note_on_at(", "note_off_at(",
    // Instrument control
    "set ", "info(", "vol(", "solo(", "mute(", "add_fx(", "add_buf(",
    "signal_from(", "change_steps(", "reset(",
    // Presets
    "load_preset(", "save_preset(", "list_presets(", "rand_preset(",
    "save_drum_part(", "load_drum_part(",
    // Scheduling
    "sched(",
    // Pattern / music helpers
    "eval_pattern(", "print_pattern(", "notes_in_chord(", "notes_in_key(",
    "print_notes(",
    // MIDI
    "midi_rec()", "midi_loop()", "midi_stop()", "midi_reset()", "midi_dump()",
    "midi_quantize(", "midi_bars(", "midi_print()",
    // Misc builtins
    "bpm(", "monitor(", "ls", "ps", "kit()", "drum_kit()", "midi_ref()",
    "lowercase(", "uppercase(", nullptr};

static char *sb_completion_generator(const char *text, int state) {
  static int list_index;
  static size_t len;
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }
  while (kCompletionWords[list_index]) {
    const char *word = kCompletionWords[list_index++];
    if (strncmp(word, text, len) == 0) return strdup(word);
  }
  return nullptr;
}

static char **sb_completion(const char *text, int start, int end) {
  (void)end;
  // When completing the first word on a line, suppress filename fallback so
  // we don't get a "display all N files?" prompt for unmatched input.
  if (start == 0) rl_attempted_completion_over = 1;
  return rl_completion_matches(text, sb_completion_generator);
}

static void RenderDisplayItem(
    const ScheduledDisplayItem &item,
    std::unordered_map<std::string, std::deque<double>> &plot_bufs) {
  std::stringstream ss;
  if (item.type == DisplayType::BAR) {
    double val = item.val;
    if (val < 0.0) val = 0.0;
    if (val > 1.0) val = 1.0;
    int filled = (int)(val * item.width);
    const char *color = val < 0.5   ? COOL_COLOR_GREEN
                        : val < 0.8 ? COOL_COLOR_YELLOW
                                    : ANSI_COLOR_RED;
    if (item.row > 0) ss << "\033[s\033[" << item.row << ";1H";
    if (!item.label.empty()) ss << item.label << " ";
    ss << "[" << color;
    for (int i = 0; i < item.width; i++) ss << (i < filled ? "█" : " ");
    char buf[16];
    snprintf(buf, sizeof(buf), "%.3f", val);
    ss << ANSI_COLOR_RESET << "] " << color << buf << ANSI_COLOR_RESET;
    if (item.row > 0)
      ss << "\033[K\033[u";
    else
      ss << "\r";
  } else {  // PLOT
    static const char *sparks[] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
    if (item.row > 0 && item.bar_pos >= 0) {
      // Waveform mode: each sample writes to its exact column within the row.
      // bar_pos 0..bar_len maps to cols 1..width on the fixed row.
      // At bar_pos==0 we clear the row first so stale content doesn't linger.
      constexpr int kBarLen = 3840;
      int label_w =
          (int)item.label.size();  // label is ASCII; size == visible cols
      int col = label_w + 1 + (item.bar_pos * item.width) / kBarLen;
      if (col < label_w + 1) col = label_w + 1;
      double v = std::min(std::max(item.val, 0.0), 1.0);
      if (item.row > max_plot_row) {
        max_plot_row = item.row;
        int rows = (global_mixr->term_rows_ > 1) ? global_mixr->term_rows_ : 24;
        printf("\033[s\033[%d;%dr\033[u", max_plot_row + 1, rows - 1);
        fflush(stdout);
      }
      if (item.bar_pos == 0) {
        // Clear the row at bar start so old-bar content doesn't linger on
        // the right while the new bar fills in left-to-right.
        std::cout << "\033[s\033[" << item.row << ";1H" << ANSI_COLOR_WHITE
                  << item.label << COOL_COLOR_GREEN << "\033[K\033[u"
                  << std::flush;
      }
      ss << "\033[s\033[" << item.row << ";" << col << "H" << COOL_COLOR_GREEN
         << sparks[(int)(v * 8)] << ANSI_COLOR_RESET << "\033[u";
    } else {
      // Scrolling deque mode: accumulate last N values and render as a bar.
      auto &buf = plot_bufs[item.label];
      buf.push_back(item.val);
      while ((int)buf.size() > item.width) buf.pop_front();
      if (item.row > 0) ss << "\033[s\033[" << item.row << ";1H";
      if (!item.label.empty()) ss << item.label << " ";
      ss << COOL_COLOR_GREEN << "[";
      for (double v : buf)
        ss << sparks[(int)(std::min(std::max(v, 0.0), 1.0) * 8)];
      ss << "]" << ANSI_COLOR_RESET << "\033[K";
      if (item.row > 0)
        ss << "\033[u";
      else
        ss << "\r";
    }
  }
  std::cout << ss.str() << std::flush;
}

int event_hook() {
  static int tick_counter = 0;
  static int current_midi_tick = 0;
  static std::vector<ScheduledDisplayItem> pending_items;
  static std::unordered_map<std::string, std::deque<double>> plot_bufs;

  // Update prompt when recording/loop state changes
  {
    static bool prev_recording = false;
    static bool prev_loop = false;
    bool cur_rec = global_mixr->midi_recording;
    bool cur_loop = global_mixr->midi_loop_;
    if (cur_rec != prev_recording || cur_loop != prev_loop) {
      if (cur_rec)
        prompt = kPromptRec;
      else if (cur_loop)
        prompt = kPromptLoop;
      else
        prompt = kPromptNormal;
      rl_set_prompt(prompt);
      cap_and_redisplay();
      prev_recording = cur_rec;
      prev_loop = cur_loop;
    }
  }

  // Handle terminal resize (SIGWINCH)
  if (terminal_resized.exchange(false)) {
    struct winsize ws = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 1) {
      int rows = static_cast<int>(ws.ws_row);
      global_mixr->term_rows_ = rows;
      printf("\033[s\033[%d;%dr\033[u", max_plot_row + 1, rows - 1);
      fflush(stdout);
      rl_resize_terminal();
      rl_set_screen_size(rows - 1,
                         ws.ws_col);  // keep readline out of status row
    } else {
      rl_resize_terminal();
    }
  }

  // Drain display_queue into pending list
  while (auto item = display_queue.try_pop()) {
    if (item) pending_items.push_back(std::move(*item));
  }

  while (auto reply = repl_queue.try_pop()) {
    if (reply) {
      const std::string &msg = *reply;
      // Check for tick message ("tick:N")
      if (msg.size() > 5 && msg.compare(0, 5, "tick:") == 0) {
        current_midi_tick = std::stoi(msg.substr(5));

        // Fire any pending display items whose time has come
        auto it = pending_items.begin();
        while (it != pending_items.end()) {
          if (it->target_tick <= current_midi_tick) {
            RenderDisplayItem(*it, plot_bufs);
            it = pending_items.erase(it);
          } else {
            ++it;
          }
        }

        // Periodic file monitoring
        if (++tick_counter < kFileCheckInterval) continue;
        tick_counter = 0;
        for (auto &f : global_mixr->file_monitors) {
          if (!f.function_file_filepath.empty()) {
            fs::path func_path = f.function_file_filepath;
            if (fs::exists(func_path)) {
              std::error_code ec;
              auto ftime = fs::last_write_time(func_path, ec);
              if (!ec) {
                if (ftime > f.function_file_filepath_last_write_time) {
                  std::string contents =
                      ReadFileContents(f.function_file_filepath);

                  // Pre-define init as NULL so the check doesn't error
                  // If file defines init, it will overwrite this
                  eval_command_queue.push("let init = NULL;");
                  eval_command_queue.push(contents);

                  // Call init() if it exists and hasn't been called yet
                  // Uses NULL pattern: _sb_init_done stays NULL until init runs
                  eval_command_queue.push(
                      "let _sb_init_done = NULL; "
                      "if (init != NULL && _sb_init_done == NULL) { init(); "
                      "_sb_init_done = true; }");

                  if (!f.initialized) {
                    f.initialized = true;
                    std::cout << "Initializing " << f.function_file_filepath
                              << std::endl;
                  } else {
                    std::cout << "Reloading " << f.function_file_filepath
                              << std::endl;
                  }

                  f.function_file_filepath_last_write_time = ftime;
                  rl_line_buffer[0] = '\0';
                  rl_done = 1;
                }
              } else {
                std::cerr << "Error opening file:" << ec << std::endl;
              }
            }
          }
        }
      } else {
        // In-place display messages end with \r, ESC-8 (DEC restore cursor),
        // or ESC-[-u (ANSI restore cursor) — skip readline redraw for these.
        bool is_inplace = !msg.empty() &&
                          (msg.back() == '\r' ||
                           (msg.size() >= 2 && msg[msg.size() - 2] == '\033' &&
                            msg.back() == '8') ||
                           (msg.size() >= 3 && msg[msg.size() - 3] == '\033' &&
                            msg[msg.size() - 2] == '[' && msg.back() == 'u'));
        if (is_inplace) {
          std::cout << msg << std::flush;
        } else {
          // Print above the current readline line without restarting readline.
          // Restarting via rl_done=1 wipes the kill ring, breaking Ctrl-U/Y/L.
          printf("\r\033[K%s\n", msg.c_str());
          rl_on_new_line();  // tell readline cursor is on a fresh line
          cap_and_redisplay();
        }
      }
    }
  }
  return 0;
}

// Expand ~ to home dir for readline history functions
static std::string expand_history_path() {
  const char *home = getenv("HOME");
  if (!home) return ".sbshell_history";
  return std::string(home) + "/.sbshell_history";
}

void *loopy() {
  std::cout << get_string_logo();

  // Reserve the top and bottom rows as fixed display areas.  Row 1 is for
  // draw_plot/draw_bar with row=1; row N is the BPM/status bar.  Scroll
  // region [2, rows-1] keeps both pinned outside the scrolling area.
  {
    struct winsize ws = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 1) {
      int rows = static_cast<int>(ws.ws_row);
      global_mixr->term_rows_ = rows;
      printf("\033[s\033[%d;%dr\033[u", max_plot_row + 1, rows - 1);
      fflush(stdout);
      rl_set_screen_size(rows - 1,
                         ws.ws_col);  // keep readline out of status row
    }
  }
  signal(SIGWINCH, handle_sigwinch);

  std::string history_path = expand_history_path();
  read_history(history_path.c_str());
  int history_base_len = history_length;  // entries loaded from disk
  setlocale(LC_ALL, "");

  std::string last_line;
  rl_event_hook = event_hook;
  rl_attempted_completion_function = sb_completion;
  rl_set_keyboard_input_timeout(500);
  rl_bind_key(' ', transport_space);
  rl_bind_key('`', transport_backtick);
  rl_bind_key('\033', transport_escape);
  rl_bind_key('\f', handle_ctrl_l);  // Ctrl-L: clear but keep cursor off row 1

  while (true) {
    std::unique_ptr<char, void (*)(void *)> line(readline(prompt), free);

    if (!line) break;  // readline returned NULL

    if (line.get() && *line.get()) {
      // Perform history expansion (!! !$ !-2 etc.) before anything else
      char *expanded = nullptr;
      int expand_result = history_expand(line.get(), &expanded);
      // expand_result: 0=no change, 1=expanded, -1=error, 2=display only
      std::unique_ptr<char, void (*)(void *)> expanded_guard(expanded, free);

      std::string current_line;
      if (expand_result == 1) {
        // Print the expanded command so the user can see what ran (bash-style)
        printf("%s\n", expanded);
        current_line = expanded;
      } else if (expand_result == 2) {
        // "display only" — print and don't execute (e.g. :p modifier)
        printf("%s\n", expanded);
        add_history(expanded);
        last_line = expanded;
        continue;
      } else if (expand_result < 0) {
        // Expansion error (e.g. !! with empty history)
        fprintf(stderr, "%s\n",
                expanded ? expanded : "history expansion error");
        continue;
      } else {
        current_line = line.get();
      }

      if (current_line != last_line) {
        add_history(current_line.c_str());
        last_line = current_line;
      }
      eval_command_queue.push(strip_line_comment(current_line));
    }
  }

  // Reset scroll region, clear screen, home cursor so bash prompt appears clean
  printf("\033[r\033[2J\033[H\033[?25h");
  fflush(stdout);

  printf(COOL_COLOR_PINK
         "Beat it, ya val jerk!\n" ANSI_COLOR_RESET);  // Thrashin'
                                                       // reference
  // Only append the new entries from this session — don't rewrite the whole
  // file, which would clobber history written by other sessions.
  int new_entries = history_length - history_base_len;
  if (new_entries > 0)
    append_history(new_entries, history_path.c_str());
  else
    write_history(history_path.c_str());  // first-ever run: create the file

  return nullptr;
}
