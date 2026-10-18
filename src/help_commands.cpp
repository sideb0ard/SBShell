#include "help_commands.h"

#include <algorithm>
#include <map>
#include <sstream>

#include "defjams.h"

// Timing reminder used throughout: 3840 ticks = 1 bar, 960 = 1 beat,
// 240 = one 16th note. Songs usually define `let pp = 240;`.

// clang-format off
static const std::vector<CommandHelp> kCommands = {
    // ─── Arrays / collections ────────────────────────────────────────────
    {"len", "Arrays", "len(arr | string | map)",
     "Number of elements, characters or map entries.", "", "len([1, 2, 3])  // 3"},
    {"head", "Arrays", "head(arr)",
     "First element (null if the array is empty).", "", "head([5, 6, 7])  // 5"},
    {"tail", "Arrays", "tail(arr)",
     "New array with everything except the first element.", "", "tail([5, 6, 7])  // [6, 7]"},
    {"last", "Arrays", "last(arr)",
     "Last element.", "", "last([5, 6, 7])  // 7"},
    {"push", "Arrays", "push(arr, item)",
     "Appends item to arr (changes arr) and returns it.", "", "push(notes, 60);"},
    {"take_n", "Arrays", "take_n(arr, n)",
     "New array with the first n elements.", "", "take_n([1, 2, 3, 4], 2)  // [1, 2]"},
    {"take_random_n", "Arrays", "take_random_n(arr, n)",
     "n elements picked at random, no element picked twice.", "",
     "take_random_n(notes_in_key(48), 3)"},
    {"reverse", "Arrays", "reverse(arr | string | midi_array)",
     "Reversed copy.", "", "reverse([1, 2, 3])  // [3, 2, 1]"},
    {"rotate", "Arrays", "rotate(arr | pattern | string, n)",
     "Rotates left by n: element n moves to the front.",
     "On a 3840-tick bar array, n below 16 counts in 16th-note steps.",
     "rotate([1, 0, 0, 0], 1)  // [0, 0, 0, 1]"},
    {"sort", "Arrays", "sort(arr)",
     "Sorts numbers ascending, in place, and returns the array.", "", "sort([3, 1, 2])  // [1, 2, 3]"},
    {"shuffle", "Arrays", "shuffle(arr)",
     "Shuffles numbers in place and returns the array.", "", "shuffle(notes);"},
    {"scramble", "Arrays", "scramble(steps16)",
     "New 16-step rhythm: each step has a 30% chance of a random hit from the input, otherwise 0.",
     "Input must have exactly 16 elements.", "let r = scramble([1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0]);"},
    {"stutter", "Arrays", "stutter(arr | pattern)",
     "Copy where steps randomly repeat, giving a stuttering feel.", "", "let s = stutter(bd_pattern);"},
    {"invert", "Arrays", "invert(arr | pattern)",
     "Swaps hits and rests: 0 becomes 1, anything else becomes 0.", "", "invert([1, 0, 0, 1])  // [0, 1, 1, 0]"},
    {"is_array", "Arrays", "is_array(val)",
     "true if val is an array.", "", ""},
    {"is_in", "Arrays", "is_in(arr, item)",
     "true if item is in arr.", "", "if (is_in([0, 4, 8], count % 16)) { ... }"},
    {"keys", "Arrays", "keys(map)",
     "Array of the map's keys.", "", "keys({\"a\": 1, \"b\": 2})"},
    {"bits", "Arrays", "bits(num)",
     "16-step array of 0/1 from num's bits, highest bit first.",
     "Handy for turning a number into a rhythm.", "bits(34952)  // [1,0,0,0, 1,0,0,0, ...]"},
    {"hex", "Arrays", "hex(num)",
     "num as a hex string.", "", "hex(255)  // \"FF\""},

    // ─── Math ────────────────────────────────────────────────────────────
    {"floor", "Math", "floor(num)", "Largest whole number <= num.", "", "floor(3.7)  // 3"},
    {"abs", "Math", "abs(num)", "Absolute value.", "", "abs(-4)  // 4"},
    {"log", "Math", "log(num)", "Base-2 logarithm, rounded down to a whole number.", "", "log(16)  // 4"},
    {"pow", "Math", "pow(base, exp)", "base to the power exp.", "", "pow(2, 3)  // 8"},
    {"sin", "Math", "sin(radians)", "Sine.", "", "sin(count * 0.1)"},
    {"cos", "Math", "cos(radians)", "Cosine.", "", "cos(count * 0.1)"},
    {"max", "Math", "max(a, b)", "Larger of two numbers.", "", "max(3, 7)  // 7"},
    {"min", "Math", "min(a, b)", "Smaller of two numbers.", "", "min(3, 7)  // 3"},
    {"incr", "Math", "incr(num, min, max)",
     "num + 1, wrapping back to min when it reaches max.",
     "max is exclusive: incr(3, 0, 4) returns 0.", "let step = incr(step, 0, 16);"},
    {"rincr", "Math", "rincr(num, min, max)",
     "num - 1, wrapping to max - 1 when it drops below min.", "", "let step = rincr(step, 0, 16);"},
    {"dincr", "Math", "dincr(num, min, max)",
     "Drunk walk: randomly num + 1 or num - 1, wrapping within min..max.", "",
     "let n = dincr(n, 0, len(notes));"},
    {"scale", "Math", "scale(val, in_min, in_max, out_min, out_max)",
     "Maps val from one range to another.", "",
     "set dx:cutoff scale(signal_from(ph), 0, 1, 200, 8000);"},

    // ─── Random / generative ─────────────────────────────────────────────
    {"rand", "Random", "rand() | rand(n) | rand(arr) | rand(gen)",
     "Random value; what it returns depends on the argument.",
     "rand()     float 0..1\n"
     "rand(1)    float 0..1\n"
     "rand(n)    whole number 0..n-1\n"
     "rand(arr)  random element of arr\n"
     "rand(gen)  randomizes the generator's sound",
     "note_on(dx, rand(notes));"},
    {"rand_array", "Random", "rand_array(length, low, high)",
     "Array of random whole numbers from low to high (inclusive).", "", "rand_array(16, 48, 60)"},
    {"rand_sixteenthz", "Random", "rand_sixteenthz(n)",
     "n different random 16th positions (0-15). With no argument, returns one.", "",
     "let hits = rand_sixteenthz(5);"},
    {"perlin", "Random", "perlin(x)",
     "Smooth noise value for x. Step x slowly for gentle random movement.", "",
     "set dx:o1rat 1 + perlin(count * 0.05);"},
    {"bjork", "Random", "bjork(pulses, steps)",
     "Euclidean rhythm: spreads pulses as evenly as possible over steps.", "",
     "bjork(3, 8)  // [1, 0, 0, 1, 0, 0, 1, 0]"},
    {"gen_perc", "Random", "gen_perc()",
     "Loads 8 random percussion samples as prc0..prc7 (random pans) plus an array percz.",
     "Takes no arguments.", "gen_perc(); note_on(prc3, 1);"},
    {"lsys_expand", "Random", "lsys_expand(axiom, rules, generations)",
     "L-system: repeatedly replaces each number i with rules[i].",
     "axiom: starting array, e.g. [0]\n"
     "rules: array of arrays; rules[i] is what symbol i expands to\n"
     "generations: 3-4 is typical, capped at 6 (and 4096 values)",
     "let seq = lsys_expand([0], [[0,2,4],[1,3,0],[2,4,1],[3,0,2],[4,1,3]], 3);"},

    // ─── Music theory ────────────────────────────────────────────────────
    {"midi_ref", "Music theory", "midi_ref()", "Prints note names and their MIDI numbers.", "", ""},
    {"notes_in_key", "Music theory", "notes_in_key(root, scale=0)",
     "MIDI notes of a scale, root to octave.",
     "scale: 0=major 1=natural minor 2=harmonic minor 3=melodic minor 4=phrygian",
     "notes_in_key(48, 1)  // C natural minor"},
    {"notes_in_chord", "Music theory", "notes_in_chord(root, key, mod=0, scale=0)",
     "Chord built on root, with its quality taken from the key and scale.",
     "mod:   0=triad 1=seventh 2=seventh inverted 3=root inverted 4=power 5=ninth\n"
     "scale: 0=major 1=natural minor 2=harmonic minor 3=melodic minor 4=phrygian",
     "notes_in_chord(50, 48)  // D minor triad in C major"},
    {"chord_notes", "Music theory", "chord_notes(root, type=0, mod=0)",
     "Chord built on root, independent of key.",
     "type: 0=major 1=minor 2=dim 3=power 4=sus2 5=sus4\n"
     "mod:  0=triad 1=min7 2=maj7 3=inverted min7 4=inverted maj7",
     "chord_notes(57, 1, 1)  // A minor 7"},
    {"scale_note", "Music theory", "scale_note(note, root, scale=0)",
     "Moves note onto the nearest note of the scale.", "scale numbers as in notes_in_key.",
     "scale_note(61, 48)  // 60 or 62 in C major"},
    {"scale_melody", "Music theory", "scale_melody(notes, root, scale=0)",
     "scale_note for every note in an array.", "", "scale_melody(rand_array(8, 48, 72), 48, 1)"},
    {"midi2note", "Music theory", "midi2note(midi_num)", "Note name for a MIDI number.", "", "midi2note(60)  // \"C4\""},
    {"midi2freq", "Music theory", "midi2freq(midi_num)", "Frequency in Hz.", "", "midi2freq(69)  // 440"},
    {"freq2midi", "Music theory", "freq2midi(hz)", "Nearest MIDI note for a frequency.", "", "freq2midi(440)  // 69"},
    {"algoz", "Music theory", "algoz()", "Prints diagrams of the FM synth algorithms.", "", ""},
    {"ratioz", "Music theory", "ratioz()", "Prints the FM synth's operator ratio table.", "", ""},

    // ─── Notes & playback ────────────────────────────────────────────────
    {"note_on", "Notes & playback", "note_on(gen, note | notes, vel=128, dur=240)",
     "Plays a note (or chord) now.",
     "dur is in ticks (240 = a 16th, 3840 = a bar).",
     "note_on(dx, [48, 52, 55], vel=100, dur=960);"},
    {"note_on_at", "Notes & playback", "note_on_at(gen, note | notes, time, vel=128, dur=240)",
     "Plays a note time ticks from now.",
     "Inside a comp's run(), time counts from the start of the bar.\n"
     "240 ticks = a 16th, 960 = a beat, 3840 = a bar.",
     "note_on_at(dx, 60, 4 * 240, dur=480);  // on the 5th 16th"},
    {"note_off", "Notes & playback", "note_off(gen, note | notes)",
     "Releases notes. note_off(gen, 0) releases all of gen's notes.", "", "note_off(dx, 0);"},
    {"note_off_at", "Notes & playback", "note_off_at(gen, note | notes, time)",
     "Releases notes time ticks from now.", "", "note_off_at(dx, 60, 960);"},
    {"stop", "Notes & playback", "stop(gen)", "Stops the generator.", "", "stop(lp);"},
    {"play_array", "Notes & playback", "play_array(gen, notes, vel=, dur=240)",
     "Spreads the array evenly across one bar and plays it.",
     "16 values = one per 16th. 0 is a rest. A nested array subdivides its slot.\n"
     "Velocity defaults to a random 110-126.",
     "play_array(dx, [48, 0, 0, 48, 0, 0, 55, 0]);"},
    {"play_array_over", "Notes & playback", "play_array_over(gen, notes, times)",
     "Plays the array `times` times within one bar (2 = twice as fast).", "",
     "play_array_over(dx, [48, 55, 60, 55], 2);"},
    {"fast", "Notes & playback", "fast(gen, notes, times, vel=, dur=)",
     "Same as play_array_over.", "", "fast(dx, [60, 63, 67], 4);"},
    {"play_rhythm", "Notes & playback", "play_rhythm({gen: notes, ...}, vel=, dur=)",
     "Plays several generators' arrays over one bar (play_array for each pair).", "",
     "play_rhythm({sbdrum: [1,0,0,0, 1,0,0,0], dx: [0,0,48,0]});"},
    {"play_map", "Notes & playback", "play_map(gen, {tick: note, ...}, vel=, dur=)",
     "Plays each note at its tick within the bar.", "", "play_map(dx, {0: 48, 960: 55, 2880: 60});"},
    {"set_pitch", "Notes & playback", "set_pitch(gen, pitch, delay=0)",
     "Sets the generator's pitch parameter, optionally delay ticks from now.", "", "set_pitch(lp, 1.5);"},
    {"speed", "Notes & playback", "speed(...)", "Not implemented yet; does nothing.", "", ""},
    {"stepn", "Notes & playback", "stepn(step_sequencer)", "Advances a step sequencer and returns its next value.", "", ""},

    // ─── Effects & mixer ─────────────────────────────────────────────────
    {"add_fx", "Mixer & effects", "add_fx(gen, \"fx\", \"fx2\", ...)",
     "Adds one or more effects to a generator's chain (addressed as gen:fx0, gen:fx1...).",
     "fx: distort lofi (bitcrush/decimate) reverb diffuser delay moddelay transverb\n"
     "    compressor sculptor gran filter modfilter djeq sidechain geometer genz\n"
     "    nnirror waveshape",
     "add_fx(dx, \"reverb\"); set dx:fx0:wetmx 0.4;"},
    {"send", "Mixer & effects", "send(\"fx\", gen | [gens], level=0.4, at=)",
     "Sends generators to a mixer effect. Note the order: effect first.",
     "fx: \"delay\", \"reverb\", \"distort\" (mixer slot 2, now the DJ EQ) or 0/1/2.",
     "send(\"reverb\", [dx, sbdrum], 0.6);"},
    {"set_send", "Mixer & effects", "set_send(gen, \"fx\", level)",
     "Changes one generator's send level. Note the order: generator first.",
     "fx: delay/dly, reverb/rev, distort/dst or 0/1/2.", "set_send(dx, \"delay\", 0.3);"},
    {"unsend", "Mixer & effects", "unsend(\"fx\", [gen | gens])",
     "Removes sends to a mixer effect: from the given generators, or from all of them.", "",
     "unsend(\"reverb\");"},
    {"global_reverb", "Mixer & effects", "global_reverb(amount)",
     "Sends the whole dry mix to the mixer reverb (0-1), on top of per-generator sends.", "",
     "sched(0, 0, 1, 3840 * 4, \"global_reverb %\");"},
    {"global_reverb_fb", "Mixer & effects", "global_reverb_fb(amount)", "Feeds reverb output back into the reverb (0-0.98).", "", ""},
    {"global_delay", "Mixer & effects", "global_delay(amount)", "Sends the whole dry mix to the mixer delay (0-1).", "", ""},
    {"global_delay_fb", "Mixer & effects", "global_delay_fb(amount)", "Feeds delay output back into the delay (0-0.98).", "", ""},
    {"global_eq", "Mixer & effects", "global_eq(amount)",
     "Runs the mix through the mixer DJ EQ. 1 = fully EQ'd, 0 = untouched.",
     "Set its cutoffs with set mixer:eq:lo <hz> and set mixer:eq:hi <hz>.",
     "global_eq(1); sched(0, 80, 2000, 3840 * 4, \"set mixer:eq:lo %\");"},
    {"global_eq_fb", "Mixer & effects", "global_eq_fb(amount)", "Feeds EQ output back into the EQ (0-0.98).", "", ""},
    {"mvol", "Mixer & effects", "mvol(volume)", "Sets the master volume.", "", "mvol(0.8);"},
    {"solo", "Mixer & effects", "solo(gen | [gens] ..., at=, dur=)",
     "Solos generators. Unsolos automatically at the next bar, or after dur ticks.", "",
     "solo(sbdrum, dur=3840 * 2);"},
    {"unsolo", "Mixer & effects", "unsolo(at=)", "Ends any solo.", "", ""},
    {"xassign", "Mixer & effects", "xassign(side, gen | [gens], at=)",
     "Puts generators on a crossfader side: 0 = left, 1 = right.", "", "xassign(0, dx); xassign(1, lp);"},
    {"xremove", "Mixer & effects", "xremove(side, gen | [gens], at=)", "Takes generators off a crossfader side.", "", ""},
    {"xclear", "Mixer & effects", "xclear(at=)", "Clears all crossfader assignments.", "", ""},
    {"xfade", "Mixer & effects", "xfade(direction, at=)",
     "Crossfades toward the left (0) or right (1) side.", "", "xfade(1, at=3840);"},

    // ─── Scheduling ──────────────────────────────────────────────────────
    {"sched", "Scheduling", "sched(when, start_val, end_val, time, \"command %\")",
     "Ramps a value from start_val to end_val, running the command with % replaced by the current value.",
     "when:     ticks from now before the ramp starts (0 = now)\n"
     "time:     how long the ramp takes, in ticks (3840 = one bar)\n"
     "commands containing \"vol\" ramp logarithmically",
     "sched(0, 0.8, 0.0, 3840, \"vol dx %\");             // fade dx out over a bar\n"
     "       sched(3840 * 4, 80, 2000, 3840 * 2, \"set dx:fx0:lo %\");  // in 4 bars, sweep over 2"},
    {"now", "Scheduling", "now()", "Current audio tick.", "", "print(now());"},
    {"timing_info", "Scheduling", "timing_info()", "Prints the mixer's timing state (tick, bar, bpm...).", "", ""},
    {"synchz", "Scheduling", "synchz(division)",
     "Frequency in Hz of a note division at the current bpm, for syncing LFOs.",
     "division: 2, 4, 8, 16 or 32 (half, quarter, 8th, 16th, 32nd notes).", "set dx:l1rate synchz(8);"},

    // ─── Patterns ────────────────────────────────────────────────────────
    {"eval_pattern", "Patterns", "eval_pattern(pattern)", "Evaluates a pattern into a 3840-step bar array.", "", ""},
    {"print_pattern", "Patterns", "print_pattern(pattern | bar_array)",
     "Prints a pattern and returns it shrunk to 16 steps.", "", ""},

    // ─── Presets & samples ───────────────────────────────────────────────
    {"list_presets", "Presets & samples", "list_presets(gen) | list_presets(drum, \"part\")",
     "Lists saved presets for a generator, or for one drum part.", "", "list_presets(sbdrum, \"sd\");"},
    {"load_preset", "Presets & samples", "load_preset(gen, \"name\")", "Loads a saved preset.", "", "load_preset(sbdrum, \"TR808\");"},
    {"save_preset", "Presets & samples", "save_preset(gen, \"name\", overwrite=false)",
     "Saves the generator's settings. Pass true as the third argument to overwrite.", "",
     "save_preset(dx, \"WOBBLE\", true);"},
    {"rand_preset", "Presets & samples", "rand_preset(gen)", "Loads a random saved preset.", "", ""},
    {"save_drum_part", "Presets & samples", "save_drum_part(drum, \"name\", \"part\")",
     "Saves one drum voice as a preset.", "part: bd sd hh oh cp fm1 fm2 fm3 lz",
     "save_drum_part(sbdrum, \"FATKICK\", \"bd\");"},
    {"load_drum_part", "Presets & samples", "load_drum_part(drum, \"name\", \"part\")",
     "Loads a saved drum-voice preset into one part.", "part: bd sd hh oh cp fm1 fm2 fm3 lz",
     "load_drum_part(sbdrum, \"FATKICK\", \"bd\");"},
    {"load_dir", "Presets & samples", "load_dir(\"dir\")",
     "Loads every sample in wavs/<dir> as a variable named after its file.", "", "load_dir(\"perc\");"},
    {"add_buf", "Presets & samples", "add_buf(looper, \"file.wav\")", "Adds another sample buffer to a looper.", "", ""},
    {"drum_kit", "Presets & samples", "drum_kit()",
     "Loads a basic sample kit: bd bdd sd cp hh ch oh shkr shkr2 rim.", "", ""},
    {"kit", "Presets & samples", "kit()",
     "Sets up a starter rig: sbdrum, sb2 (drumsynths), dx, dx2, dx3 (FM) and mo (moog).", "", ""},
    {"monitor", "Presets & samples", "monitor(\"file.sb\")", "Live-reloads a script file whenever it changes.", "", ""},

    // ─── MIDI ────────────────────────────────────────────────────────────
    {"midi_init", "MIDI", "midi_init()", "Connects to MIDI input.", "", ""},
    {"midi_assign", "MIDI", "midi_assign(gen)", "Routes the MIDI controller to a generator.", "", ""},
    {"midi_map", "MIDI", "midi_map(cc, \"param\") | midi_map()",
     "Binds a MIDI CC number to a parameter. With no arguments, prints the map.", "", "midi_map(74, \"cutoff\");"},
    {"midi_rec", "MIDI", "midi_rec()", "Toggles MIDI recording.", "", ""},
    {"midi_print", "MIDI", "midi_print()", "Toggles printing incoming MIDI.", "", ""},
    {"midi_reset", "MIDI", "midi_reset()", "Clears the recording.", "", ""},
    {"midi_stop", "MIDI", "midi_stop()", "Stops MIDI recording/playback.", "", ""},
    {"midi_bars", "MIDI", "midi_bars(n)", "Sets the recording length in bars.", "", ""},
    {"midi_loop", "MIDI", "midi_loop()", "Toggles looping playback of the recording.", "", ""},
    {"midi_quantize", "MIDI", "midi_quantize(divisions=16)", "Quantizes the recording to divisions per bar.", "", ""},
    {"midi_dump", "MIDI", "midi_dump()", "Returns the recording as a midi array.", "", "let rec = midi_dump();"},
    {"midi2array", "MIDI", "midi2array(midi_array)",
     "Converts a midi array to [notes, durations], each an array of 16-step bars.", "", ""},
    {"midi_at", "MIDI", "midi_at(midi_array, step)", "Note at a 16th step (0 if none).", "", ""},
    {"midi_fix", "MIDI", "midi_fix(midi_array)", "Snaps a midi array's events to 32nd notes, in place.", "", ""},
    {"websock", "MIDI", "websock(true)", "Enables the websocket server.", "", ""},

    // ─── Phasor ──────────────────────────────────────────────────────────
    {"signal_from", "Phasor", "signal_from(phasor)", "Next value from a phasor.", "", ""},
    {"change_steps", "Phasor", "change_steps(phasor, steps)", "Changes a phasor's step count.", "", ""},
    {"reset", "Phasor", "reset(phasor)", "Resets a phasor to its start.", "", ""},

    // ─── Display & utility ───────────────────────────────────────────────
    {"print", "Utility", "print(val, ...)", "Prints its arguments, joined together. Alias: puts.", "", "print(\"step:\", step);"},
    {"puts", "Utility", "puts(val, ...)", "Same as print.", "", ""},
    {"type", "Utility", "type(val)", "Type name of a value.", "", "type([1])  // \"ARRAY\""},
    {"lowercase", "Utility", "lowercase(str)", "Lowercase copy.", "", ""},
    {"uppercase", "Utility", "uppercase(str)", "Uppercase copy.", "", ""},
    {"funcz", "Utility", "funcz()", "Lists defined functions and generators.", "", ""},
    {"draw_bar", "Utility", "draw_bar(val, width=40, \"label\", at=, row=)",
     "Draws a 0-1 value as a bar in the terminal.",
     "at=: tick within the bar to draw at. row=: draw that many lines up.",
     "draw_bar(signal_from(ph), \"lfo\");"},
    {"draw_plot", "Utility", "draw_plot(val, width=40, \"label\", at=, row=)",
     "Draws a scrolling sparkline of 0-1 values (one history per label).", "",
     "draw_plot(signal_from(ph), 60, \"lfo\");"},
};
// clang-format on

const std::vector<CommandHelp>& CommandHelpTable() {
  return kCommands;
}

const CommandHelp* FindCommandHelp(const std::string& name) {
  for (const auto& c : kCommands)
    if (name == c.name) return &c;
  return nullptr;
}

static void IndentLines(std::stringstream& ss, const std::string& text,
                        const char* indent) {
  std::stringstream in(text);
  std::string line;
  while (std::getline(in, line)) ss << indent << line << "\n";
}

std::string FormatCommandHelp(const CommandHelp& help) {
  const CommandHelp& c = help;
  std::stringstream ss;
  ss << "\n"
     << COOL_COLOR_GREEN << c.name << ANSI_COLOR_RESET << "  "
     << COOL_COLOR_ORANGE << c.summary << "\n\n";
  ss << ANSI_COLOR_WHITE << "  " << c.usage << "\n";
  if (*c.details) {
    ss << "\n" << COOL_COLOR_ORANGE;
    IndentLines(ss, c.details, "  ");
  }
  if (*c.example) {
    ss << "\n"
       << COOL_COLOR_GREEN << "  e.g. " << ANSI_COLOR_WHITE << c.example
       << "\n";
  }
  ss << ANSI_COLOR_RESET << "\n";
  return ss.str();
}

std::string FormatCommandIndex() {
  // Keep categories in table order.
  std::vector<std::string> order;
  std::map<std::string, std::vector<std::string>> by_cat;
  for (const auto& c : kCommands) {
    if (!by_cat.count(c.category)) order.push_back(c.category);
    by_cat[c.category].push_back(c.name);
  }

  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nCommands" << ANSI_COLOR_RESET
     << COOL_COLOR_ORANGE << "  (help <name> for details)\n\n";
  for (const auto& cat : order) {
    ss << COOL_COLOR_GREEN << cat << "\n" << ANSI_COLOR_WHITE;
    int col = 0;
    for (const auto& name : by_cat[cat]) {
      if (col + name.size() + 2 > 76) {
        ss << "\n";
        col = 0;
      }
      if (col == 0) {
        ss << "  ";
        col = 2;
      }
      ss << name << "  ";
      col += name.size() + 2;
    }
    ss << "\n\n";
  }
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

std::vector<std::string> SimilarCommands(const std::string& fragment) {
  std::vector<std::string> out;
  if (fragment.empty()) return out;
  for (const auto& c : kCommands)
    if (std::string(c.name).find(fragment) != std::string::npos)
      out.push_back(c.name);
  return out;
}
