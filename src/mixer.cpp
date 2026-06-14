#include <audio_action_queue.h>
#include <defjams.h>
#include <drum_synth.h>
#include <drumsampler.h>
#include <event_queue.h>
#include <fmsynth.h>
#include <fx/distortion.h>
#include <fx/envelope.h>
#include <fx/fx.h>
#include <fx/reverb.h>
#include <granular_looper.h>
#include <mixer.h>
#include <obliquestrategies.h>
#include <portaudio.h>
#include <portmidi.h>
#include <soundgenerator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <subsynth.h>
#include <unistd.h>
#include <utils.h>

#include <cmath>
#include <deque>
#include <filereader.hpp>
#include <interpreter/object.hpp>
#include <interpreter/sound_cmds.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <tsqueue.hpp>

namespace {

std::vector<std::string> env_names = {"att", "dec", "rel"};

std::pair<double, double> GetBoundaries(const std::string &param) {
  if (param.find("pct") != std::string::npos) return std::make_pair(0, 100);
  if (std::any_of(env_names.begin(), env_names.end(),
                  [&param](const std::string &p) {
                    return param.find(p) != std::string::npos;
                  })) {
    return std::make_pair(1, 5000);
  }
  if (param.find("fc") != std::string::npos) return std::make_pair(80, 1800);
  if (param.find("fq") != std::string::npos) return std::make_pair(0.5, 9.7);
  if (param.find("det") != std::string::npos) return std::make_pair(-100, 100);
  if (param.find("int") != std::string::npos) return std::make_pair(-1, 1);
  if (param.find("rate") != std::string::npos) return std::make_pair(0.02, 20);
  if (param.find("ratio") != std::string::npos)
    return std::make_pair(-0.99, 0.99);
  if (param.find("rat") != std::string::npos) return std::make_pair(0.1, 26);
  if (param.find("out") != std::string::npos) return std::make_pair(0, 100);
  if (param.find("algo") != std::string::npos) return std::make_pair(0, 7);
  if (param.find("grain") != std::string::npos) return std::make_pair(0, 100);
  if (param.find("wav") != std::string::npos) return std::make_pair(0, 7);
  if (param.find("porta") != std::string::npos) return std::make_pair(0, 5000);
  if (param.find("delayms") != std::string::npos)
    return std::make_pair(0, 2000);
  if (param.find("fb") != std::string::npos) return std::make_pair(0, 99);
  if (param.find("wetmx") != std::string::npos) return std::make_pair(0, 0.99);

  return std::make_pair(0, 1);
}

}  // namespace

std::unique_ptr<Mixer> global_mixr;
extern std::shared_ptr<object::Environment> global_env;
extern Tsqueue<event_queue_item> process_event_queue;
extern Tsqueue<std::string> repl_queue;
extern Tsqueue<std::unique_ptr<AudioActionItem>> audio_queue;
extern Tsqueue<int> audio_reply_queue;
extern Tsqueue<std::string> eval_command_queue;

Action::Action(double start_val, double end_val, int time_taken_ticks,
               std::string action_to_take)
    : start_val_{start_val},
      end_val_{end_val},
      cur_val_{start_val},
      time_taken_ticks_{time_taken_ticks},
      action_to_take_{action_to_take} {
  // Detect if this is a volume action - use logarithmic interpolation
  if (action_to_take_.find("vol") != std::string::npos) {
    interpolation_type_ = InterpolationType::Logarithmic;
  }

  if (time_taken_ticks_ == 0) {
    incr_ = 1;
  } else {
    double diff = std::abs(start_val_ - end_val_);
    incr_ = diff / time_taken_ticks_;
  }
  if (start_val_ > end_val_) {
    dir_ = 0;  // downward
    incr_ *= -1;
  }
  if (incr_ == 0) {
    std::cout << "Nah mate, nae zeros allowed!\n";
  } else {
    active_ = true;
  }
}

void Action::Run() {
  if (active_) {
    elapsed_ticks_++;

    if (interpolation_type_ == InterpolationType::Logarithmic) {
      // Exponential interpolation for volume (logarithmic perception)
      if (elapsed_ticks_ >= time_taken_ticks_) {
        cur_val_ = end_val_;
        active_ = false;
      } else if (start_val_ > 0 && end_val_ > 0) {
        // Safe to use exponential interpolation
        double progress =
            static_cast<double>(elapsed_ticks_) / time_taken_ticks_;
        double ratio = end_val_ / start_val_;
        cur_val_ = start_val_ * std::pow(ratio, progress);
      } else {
        // Fall back to linear if either value is 0 or negative
        cur_val_ += incr_;
        if ((dir_ && (cur_val_ >= end_val_)) ||
            (!dir_ && (cur_val_ <= end_val_))) {
          cur_val_ = end_val_;
          active_ = false;
        }
      }
    } else {
      // Linear interpolation (default)
      cur_val_ += incr_;
      if ((dir_ && (cur_val_ >= end_val_)) ||
          (!dir_ && (cur_val_ <= end_val_))) {
        cur_val_ = end_val_;
        active_ = false;
      }
    }

    std::string new_cmd =
        ReplaceString(action_to_take_, "%", std::to_string(cur_val_));
    eval_command_queue.try_push(new_cmd);
  }
}

Mixer::Mixer() {
  UpdateBpm(DEFAULT_BPM);
  solo_gain_.fill(1.0);
  solo_gain_target_.fill(1.0);

  for (int i = 0; i < MAX_NUM_PROC; i++)
    processes_[i] = std::make_shared<Process>();
  proc_initialized_ = true;

  fx_[0] = std::make_shared<StereoDelay>();
  fx_[1] = std::make_shared<Reverb>();
  fx_[2] = std::make_shared<Distortion>();

  // the lifetime of these booleans is a single sample
  timing_info.cur_sample = -1;
  timing_info.midi_tick = -1;
  timing_info.sixteenth_note_tick = -1;
  timing_info.loop_beat = 0;
  timing_info.time_of_next_midi_tick = 0;
  timing_info.has_started = false;
  timing_info.is_midi_tick = true;
  timing_info.is_start_of_loop = true;
  timing_info.is_end_of_loop = false;
  timing_info.is_thirtysecond = true;
  timing_info.is_twentyfourth = true;
  timing_info.is_sixteenth = true;
  timing_info.is_twelth = true;
  timing_info.is_eighth = true;
  timing_info.is_sixth = true;
  timing_info.is_quarter = true;
  timing_info.is_third = true;

  std::string contents = ReadFileContents(kStartupConfigFile);
  eval_command_queue.push(contents);
}

std::string Mixer::StatusMixr() {
  //  clang-format off
  std::stringstream ss;

  ss << COOL_COLOR_GREEN << ":::::::::::: vol:" << ANSI_COLOR_WHITE << volume
     << COOL_COLOR_GREEN << " bpm:" << ANSI_COLOR_WHITE << bpm
     << COOL_COLOR_GREEN << " looplen:" << ANSI_COLOR_WHITE << 3840
     << COOL_COLOR_GREEN << " midi_device:" << ANSI_COLOR_WHITE
     << (have_midi_controller ? "true" : "false") << COOL_COLOR_GREEN
     << " websock:" << ANSI_COLOR_WHITE
     << (websocket_enabled_ ? "true" : "false") << COOL_COLOR_GREEN
     << " :::::::::::::::::\n";

  if (fx_[0]) {
    ss << COOL_COLOR_GREEN << ":::::::::::: " << COOL_COLOR_ORANGE
       << "delay: " << fx_[0]->Status() << std::endl;
  }
  if (fx_[1]) {
    ss << COOL_COLOR_GREEN << ":::::::::::: " << COOL_COLOR_ORANGE
       << "reverb: " << fx_[1]->Status() << std::endl;
  }
  if (fx_[2]) {
    ss << COOL_COLOR_GREEN << ":::::::::::: " << COOL_COLOR_ORANGE
       << "distort: " << fx_[2]->Status() << std::endl;
  }

  if (global_env) {
    ss << COOL_COLOR_GREEN << ":::::::::::: " << COOL_COLOR_ORANGE
       << "xfader: " << xfader_.Status(global_env->GetSoundGeneratorsById())
       << std::endl;
  }
  ss << ANSI_COLOR_WHITE;
  // clang-format on

  return ss.str();
}

std::string Mixer::StatusProcz(bool all) {
  std::stringstream ss;
  ss << COOL_COLOR_ORANGE << "\n[" << ANSI_COLOR_WHITE << "Procz"
     << COOL_COLOR_ORANGE << "]\n";

  for (int i = 0; i < MAX_NUM_PROC; i++) {
    // Make a copy of the shared_ptr to avoid races
    auto p = processes_[i];
    if (!p) continue;  // Skip if null

    // Check if we should display this process
    if (p->active_ || all) {
      ss << ANSI_COLOR_WHITE << "p" << i << ANSI_COLOR_RESET << " "
         << p->Status() << std::endl;
    }
  }

  return ss.str();
}
std::string Mixer::StatusEnv() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\n[" << ANSI_COLOR_WHITE << "Varz"
     << COOL_COLOR_GREEN << "]" << std::endl;

  if (!global_env) {
    ss << ANSI_COLOR_RED << "ERROR: global_env is null!" << ANSI_COLOR_RESET
       << std::endl;
    return ss.str();
  }

  ss << global_env->Debug();

  // Get the name->index mapping (this doesn't need the sg mutex)
  std::map<std::string, int> soundgens = global_env->GetSoundGeneratorsByName();

  // Read from cached status - no need to lock sound_generators_mutex_
  std::lock_guard<std::mutex> lock(cached_status_mutex_);

  for (auto &[var_name, sg_idx] : soundgens) {
    if (sg_idx >= 0 && sg_idx < cached_sg_count_.load()) {
      ss << ANSI_COLOR_WHITE << var_name << ANSI_COLOR_RESET " = ";

      size_t margin_size = var_name.length() + 3;  // " = "
      std::string margin(margin_size, ' ');

      auto &cached = cached_sg_status_[sg_idx];

      std::stringstream sgss(cached.status);
      std::string sg_status_line;
      bool first_line = true;
      while (std::getline(sgss, sg_status_line)) {
        if (!first_line) {
          ss << margin;
        }
        ss << sg_status_line << std::endl;
        first_line = false;
      }

      // Read from cached FX status
      for (int i = 0; i < cached.effects_num; i++) {
        ss << margin << ANSI_COLOR_WHITE << "fx" << i << " ";
        if (cached.fx[i].enabled)
          ss << COOL_COLOR_YELLOW;
        else
          ss << ANSI_COLOR_RESET;

        size_t fx_margin_size = margin.length() + 4;
        std::string fx_margin(fx_margin_size, ' ');
        std::stringstream fxss(cached.fx[i].status);
        std::string status_line;
        bool first_line = true;
        while (std::getline(fxss, status_line)) {
          if (!first_line) {
            ss << fx_margin;
          }
          ss << status_line << std::endl;
          first_line = false;
        }
      }
      ss << ANSI_COLOR_RESET;
    }
  }
  return ss.str();
}

std::string Mixer::StatusSgz(bool all) {
  std::stringstream ss;

  // Read from cached status - no need to lock sound_generators_mutex_
  std::lock_guard<std::mutex> lock(cached_status_mutex_);

  int count = cached_sg_count_.load();
  if (count > 0) {
    ss << COOL_COLOR_GREEN << "\n[" << ANSI_COLOR_WHITE << "sound generators"
       << COOL_COLOR_GREEN << "]\n";

    for (int i = 0; i < count; i++) {
      auto &cached = cached_sg_status_[i];
      if ((cached.active && cached.volume > 0.0) || all) {
        ss << COOL_COLOR_GREEN << "[" << ANSI_COLOR_WHITE << "s" << i
           << COOL_COLOR_GREEN << "] " << ANSI_COLOR_RESET;
        ss << cached.info << ANSI_COLOR_RESET << "\n";

        if (cached.effects_num > 0) {
          ss << "      ";
          for (int j = 0; j < cached.effects_num; j++) {
            if (cached.fx[j].enabled)
              ss << COOL_COLOR_YELLOW;
            else
              ss << ANSI_COLOR_RESET;
            ss << "\n[fx " << i << ":" << j << cached.fx[j].status << "]";
          }
          ss << ANSI_COLOR_RESET;
        }
        ss << "\n\n";
      }
    }
  }
  return ss.str();
}

void Mixer::PrintFuncAndGenInfo() {
  std::stringstream ss;
  ss << global_env->ListFuncsAndGen();
  repl_queue.push(ss.str());
}

void Mixer::Ps(bool all) {
  std::stringstream ss;
  ss << get_string_logo();
  ss << StatusMixr();
  ss << StatusEnv();
  ss << StatusProcz();
  // ss << mixer_status_procz(mixr, all);
  ss << ANSI_COLOR_RESET;

  if (all) ss << StatusSgz(all);

  repl_queue.push(ss.str());
}

void Mixer::Now() {
  audio_reply_queue.push(timing_info.midi_tick);
}

void Mixer::EmitEvent(broadcast_event event) {
  event_queue_item ev;
  ev.type = Event::TIMING_EVENT;
  ev.timing_info = timing_info;
  process_event_queue.push(ev);

  std::shared_lock<std::shared_mutex> lock(
      sound_generators_mutex_);  // Shared lock for read
  for (int i = 0; i < sound_generators_idx_; i++) {
    auto &sg = sound_generators_[i];
    if (sg) {
      sg->EventNotify(event, timing_info);
      if (sg->effects_num > 0) {
        // Shared lock to safely read effects array
        std::shared_lock<std::shared_mutex> fx_lock(sg->effects_mutex_);
        for (int j = 0; j < sg->effects_num; j++) {
          if (sg->effects_[j]) {
            auto f = sg->effects_[j];
            f->EventNotify(event, timing_info);
          }
        }
      }
    }
  }
}

void Mixer::UpdateBpm(int new_bpm) {
  bpm = new_bpm;
  bpm_to_be_updated = new_bpm;
  timing_info.bpm = bpm;
  // timing_info.frames_per_midi_tick = (60.0 / bpm * SAMPLE_RATE) / PPQN;
  timing_info.frames_per_midi_tick = (bpm / 60.0 * SAMPLE_RATE) / PPQN;
  timing_info.loop_len_in_frames = timing_info.frames_per_midi_tick * PPBAR;
  timing_info.loop_len_in_ticks = PPBAR;

  // hmm, not sure if these are correct!
  timing_info.ms_per_midi_tick = 60000.0 / (bpm * PPQN);
  timing_info.midi_ticks_per_ms = PPQN / (60000.0 / bpm);

  timing_info.size_of_thirtysecond_note =
      (PPSIXTEENTH / 2) * timing_info.frames_per_midi_tick;
  timing_info.size_of_sixteenth_note =
      timing_info.size_of_thirtysecond_note * 2;
  timing_info.size_of_eighth_note = timing_info.size_of_sixteenth_note * 2;
  timing_info.size_of_quarter_note = timing_info.size_of_eighth_note * 2;

  EmitEvent((broadcast_event){.type = TIME_BPM_CHANGE, .sequencer_src = 0});
}

void Mixer::VolChange(float vol) {
  if (vol >= 0.0 && vol <= 1.0) {
    volume = vol;
  }
}

void Mixer::VolChange(int sg, float vol) {
  if (!IsValidSoundgenNum(sg)) {
    printf("Nah mate, returning\n");
    return;
  }
  sound_generators_[sg]->SetVolume(vol);
}

void Mixer::PanChange(int sg, float val) {
  if (!IsValidSoundgenNum(sg)) {
    printf("Nah mate, returning\n");
    return;
  }
  sound_generators_[sg]->SetPan(val);
}

void Mixer::AddSoundGenerator(std::unique_ptr<SBAudio::SoundGenerator> sg) {
  std::unique_lock<std::shared_mutex> lock(
      sound_generators_mutex_);  // Exclusive lock for write
  int soundgen_id = sound_generators_idx_;
  if (soundgen_id < MAX_NUM_SOUND_GENERATORS) {
    sg->soundgen_id_ = soundgen_id;
    sound_generators_[sound_generators_idx_++] = std::move(sg);
  } else {
    std::cout << "NAH, FULL MATe!\n";
    soundgen_id = -1;
  }
  audio_reply_queue.push(soundgen_id);
}

void Mixer::MidiTick() {
  timing_info.is_thirtysecond = false;
  timing_info.is_twentyfourth = false;
  timing_info.is_sixteenth = false;
  timing_info.is_twelth = false;
  timing_info.is_eighth = false;
  timing_info.is_sixth = false;
  timing_info.is_quarter = false;
  timing_info.is_third = false;
  timing_info.is_start_of_loop = false;
  timing_info.is_end_of_loop = false;

  int cur_tick = timing_info.midi_tick % PPBAR;
  if (cur_tick == 0) {
    timing_info.is_start_of_loop = true;
  }
  if (cur_tick == PPBAR - 1) {
    timing_info.is_end_of_loop = true;
  }

  if (timing_info.midi_tick % 120 == 0) {
    timing_info.is_thirtysecond = true;

    if (timing_info.midi_tick % 240 == 0) {
      timing_info.is_sixteenth = true;
      timing_info.sixteenth_note_tick++;

      if (timing_info.midi_tick % 480 == 0) {
        timing_info.is_eighth = true;

        if (timing_info.midi_tick % PPQN == 0) {
          timing_info.is_quarter = true;
        }
      }
    }
  }

  CheckForAudioActionQueueMessages();
  CheckForExternalMidiEvents();

  if (midi_loop_ && !midi_recording) {
    PlaybackMidiLoopTick();
  }

  midi_tick_ = timing_info.midi_tick;
  repl_queue.push("tick:" + std::to_string(timing_info.midi_tick));
  if (timing_info.is_sixteenth) EmitTimingDisplay();
  EmitEvent((broadcast_event){.type = TIME_MIDI_TICK, .sequencer_src = 0});
  // lo_send(processing_addr, "/bpm", NULL);
  CheckForDelayedEvents();

  RunScheduledActions();

  // Update cached status on every beat for lock-free status reads
  if (timing_info.is_quarter) {
    UpdateCachedStatus();
  }
}

// GenNext implementation moved to header as template function

void Mixer::UpdateCachedStatus() {
  // Called from audio thread on every beat - updates cached status info
  // Uses try_lock to never block the audio thread
  if (!cached_status_mutex_.try_lock()) {
    return;  // Skip this update if UI thread is reading
  }

  // We have the sound_generators_mutex_ shared lock from GenNext context
  // but we're in MidiTick which doesn't hold it, so we need to acquire it
  if (!sound_generators_mutex_.try_lock_shared()) {
    cached_status_mutex_.unlock();
    return;  // Skip if we can't get the lock
  }

  int count = sound_generators_idx_.load();
  for (int i = 0; i < count; i++) {
    auto &sg = sound_generators_[i];
    if (sg) {
      cached_sg_status_[i].info = sg->Info();
      cached_sg_status_[i].status = sg->Status();
      cached_sg_status_[i].active = sg->active;
      cached_sg_status_[i].volume = sg->GetVolume();
      cached_sg_status_[i].effects_num = sg->effects_num.load();

      // Cache FX status - try to get the effects lock
      if (sg->effects_mutex_.try_lock_shared()) {
        for (int j = 0; j < sg->effects_num; j++) {
          if (sg->effects_[j]) {
            cached_sg_status_[i].fx[j].status = sg->effects_[j]->Status();
            cached_sg_status_[i].fx[j].enabled = sg->effects_[j]->enabled_;
          }
        }
        sg->effects_mutex_.unlock_shared();
      }
    }
  }
  cached_sg_count_.store(count);

  sound_generators_mutex_.unlock_shared();
  cached_status_mutex_.unlock();
}

bool Mixer::DelSoundgen(int soundgen_num) {
  std::unique_lock<std::shared_mutex> lock(
      sound_generators_mutex_);  // Exclusive lock for write
  if (IsValidSoundgenNum(soundgen_num)) {
    printf("MIXR!! Deleting SOUND GEN %d\n", soundgen_num);
    sound_generators_[soundgen_num] = nullptr;
  }
  return true;
}

bool Mixer::IsValidSoundgenNum(int sg_num) {
  if (sg_num >= 0 && sg_num < sound_generators_idx_ &&
      sg_num < static_cast<int>(sound_generators_.size()) &&
      sound_generators_[sg_num])
    return true;

  return false;
}

bool Mixer::IsValidFx(int soundgen_num, int fx_num) {
  if (IsValidSoundgenNum(soundgen_num)) {
    const auto &sg = sound_generators_[soundgen_num];
    // Shared lock to safely read effects array
    std::shared_lock<std::shared_mutex> lock(sg->effects_mutex_);
    if (fx_num >= 0 && fx_num < sg->effects_num && sg->effects_[fx_num])
      return true;
  }
  return false;
}

void Mixer::PrintDxAlgos() {
  std::stringstream ss;
  ss << ANSI_COLOR_WHITE "DX Algos:\n";
  ss << "  0.    1.    2.      3.    4.     5.        6.        7.\n\n";
  ss << "  4\n";
  ss << "  |\n";
  ss << "  3     3 4   3       4\n";
  ss << "  |     \\ |   |       |\n";
  ss << "  2       2   2 4   2 3   2  4      4            4\n";
  ss << "  |       |   |/    |/    |  |    / | \\          |\n";
  ss << "  1       1   1     1     1__3   1__2__3   1__2__3   1__2__3__4\n";
  ss << ANSI_COLOR_RESET;
  repl_queue.push(ss.str());
}
void Mixer::PrintDxRatioz() {
  std::stringstream ss;
  ss << ANSI_COLOR_WHITE "DX Ratioz:\n";
  ss << "0.5,   0.71,  0.78,  0.87,  1.0,   1.41,  1.57,  1.73,  2.,    2.82,  "
        "3.,\n";
  ss << "3.14,  3.46,  4.,    4.24,  4.71,  5.,    5.19,  5.65,  6.0,   6.28,  "
        "6.92,\n";
  ss << "7.,    7.07,  7.85,  8.0,   8.48,  8.65,  9.0,   9.42,  9.89,  10.,   "
        "10.38,\n";
  ss << "10.99, 11.0,  11.30, 12.0,  12.11, 12.56, 12.72, 13.0,  13.84, 14.0,  "
        "14.1,\n";
  ss << "14.13, 15.,   15.55, 15.37, 15.70, 16.96, 17.27, 17.30, 18.37, 18.84, "
        "19.03,\n";
  ss << "19.78, 20.41, 20.76, 21.20, 21.98, 22.49, 23.53, 24.22, 25.95\n";
  ss << ANSI_COLOR_RESET;
  repl_queue.push(ss.str());
}

void Mixer::PrintTimingInfo() {
  const mixer_timing_info *info = &timing_info;
  printf("TIMING INFO!\n");
  printf("============\n");
  printf("FRAMES per midi tick:%f\n", info->frames_per_midi_tick);
  printf("MS per MIDI tick:%f\n", info->ms_per_midi_tick);
  printf("TIME of next MIDI tick:%f\n", info->time_of_next_midi_tick);
  printf("SIXTEENTH NOTE tick:%d\n", info->sixteenth_note_tick);
  printf("MIDI tick:%d\n", info->midi_tick);
  printf("CUR SAMPLE:%d\n", info->cur_sample);
  printf("Loop_len_in_ticks:%f\n", info->loop_len_in_ticks);
  printf("Has_started:%d\n", info->has_started);
  printf("Start of loop:%d\n", info->is_start_of_loop);
  printf("Is midi_tick:%d\n", info->is_midi_tick);
}

// called from Process or Repl thread
void Mixer::ScheduleAction(int when, Action item) {
  std::lock_guard<std::mutex> lock(scheduled_actions_mutex_);
  scheduled_actions_.insert(std::make_pair(timing_info.midi_tick + when, item));
}

void Mixer::RunScheduledActions() {
  int now = timing_info.midi_tick;
  if (scheduled_actions_mutex_.try_lock()) {
    for (auto it = scheduled_actions_.begin();
         it != scheduled_actions_.end();) {
      if (now >= it->first) {
        running_actions_.push_back(it->second);
        scheduled_actions_.erase(it++);
      } else {
        ++it;
      }
    }
    scheduled_actions_mutex_.unlock();
  }

  for (auto &a : running_actions_) {
    a.Run();
  }

  std::erase_if(running_actions_, [](Action a) { return !a.IsActive(); });
}

double Mixer::GetHzPerBar() {
  double hz_per_beat = (60. / bpm);
  return hz_per_beat / 4;
}

double Mixer::GetHzPerTimingUnit(unsigned int timing_unit) {
  double return_val = 0;
  double hz_per_beat = (60. / bpm);
  if (timing_unit == Quantize::Q2) return_val = hz_per_beat / 2.;
  if (timing_unit == Quantize::Q4)
    return_val = hz_per_beat;
  else if (timing_unit == Quantize::Q8)
    return_val = hz_per_beat * 2;
  else if (timing_unit == Quantize::Q16)
    return_val = hz_per_beat * 4;
  else if (timing_unit == Quantize::Q32)
    return_val = hz_per_beat * 8;

  return return_val;
}

int Mixer::GetTicksPerCycleUnit(unsigned int event_type) {
  int ticks = 0;
  switch (event_type) {
    case (TIME_START_OF_LOOP_TICK):
      ticks = timing_info.loop_len_in_ticks;
      break;
    case (TIME_MIDI_TICK):
      ticks = 1;
      break;
    case (TIME_QUARTER_TICK):
      ticks = PPQN;
      break;
    case (TIME_EIGHTH_TICK):
      ticks = PPQN / 2;
      break;
    case (TIME_SIXTEENTH_TICK):
      ticks = PPQN / 4;
      break;
    case (TIME_THIRTYSECOND_TICK):
      ticks = PPQN / 8;
      break;
  }
  return ticks;
}

void Mixer::PreviewAudio(std::unique_ptr<AudioActionItem> action) {
  if (action->buffer.size() > 0) {
    preview.filename = action->preview_filename;
    preview.audio_buffer_len = action->audio_buffer_details.buffer_length;
    preview.audio_buffer = std::move(action->buffer);
    preview.num_channels = action->audio_buffer_details.num_channels;
    preview.audio_buffer_read_idx = 0;
    preview.enabled = true;
  }
}

StereoVal PreviewBuffer::Generate() {
  StereoVal ret = {.0, .0};
  if (!enabled) return ret;

  if (audio_buffer_read_idx < static_cast<int>(audio_buffer.size())) {
    ret.left = audio_buffer[audio_buffer_read_idx];
    if (num_channels == 1)
      ret.right = ret.left;
    else
      ret.right = audio_buffer[audio_buffer_read_idx + 1];

    audio_buffer_read_idx += num_channels;
    if (audio_buffer_read_idx >= audio_buffer_len) {
      audio_buffer_read_idx = 0;
      enabled = false;
    }
  }

  return ret;
}

void Mixer::ProcessActionMessage(std::unique_ptr<AudioActionItem> action) {
  if (action->type == AudioAction::STATUS) {
    Ps(action->status_all);
  }
  if (action->type == AudioAction::MIDI_MAP)
    AddMidiMapping(action->mapped_id, action->mapped_param);
  else if (action->type == AudioAction::ENABLE_WEBSOCKET) {
    std::cout << "GENERAL VAL:" << action->general_val << " is trei>?"
              << std::endl;
    EnableWebSocket(action->general_val);
  } else if (action->type == AudioAction::MIDI_MAP_SHOW)
    PrintMidiMappings();
  else if (action->type == AudioAction::MONITOR) {
    AddFileToMonitor(action->filepath);
  } else if (action->type == AudioAction::ADD) {
    if (action->sg) {
      AddSoundGenerator(std::move(action->sg));
    }
  } else if (action->type == AudioAction::ADD_FX) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      auto &sg = sound_generators_[action->soundgen_num];
      if (sg) {
        for (auto fx : action->fx) {
          sg->AddFx(fx);
        }
      }
    }
    audio_reply_queue.push(1);  // unblock add_fx() caller
  } else if (action->type == AudioAction::ADD_BUFFER) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      auto &sg = sound_generators_[action->soundgen_num];
      if (sg && (sg->type == LOOPER_TYPE || sg->type == WAVSYNTH_TYPE)) {
        sg->AddBuffer(std::move(action->fb));
      }
    }
  } else if (action->type == AudioAction::BPM) {
    UpdateBpm(action->new_bpm);
  } else if (action->type == AudioAction::NOW) {
    Now();
  } else if (action->type == AudioAction::VOLUME) {
    VolChange(action->new_volume);
  } else if (action->type == AudioAction::MIDI_NOTE_ON_DELAYED) {
    // Schedule note on via delayed_action_items_
    int on_tick = timing_info.midi_tick + action->note_start_time;
    auto note_on = std::make_unique<AudioActionItem>(AudioAction::MIDI_NOTE_ON);
    note_on->soundgen_num = action->soundgen_num;
    note_on->notes = action->notes;
    note_on->velocity = action->velocity;
    note_on->duration = 0;  // prevent recursive scheduling
    note_on->start_at = on_tick;
    delayed_action_items_.push_back(std::move(note_on));
    if (action->duration) {
      auto note_off =
          std::make_unique<AudioActionItem>(AudioAction::MIDI_NOTE_OFF);
      note_off->soundgen_num = action->soundgen_num;
      note_off->notes = action->notes;
      note_off->velocity = action->velocity;
      note_off->start_at = on_tick + action->duration;
      delayed_action_items_.push_back(std::move(note_off));
    }
  } else if (action->type == AudioAction::MIDI_NOTE_ON) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      auto &sg = sound_generators_[action->soundgen_num];
      for (auto midinum : action->notes) {
        midi_event event_on =
            new_midi_event(MIDI_ON, midinum, action->velocity);
        event_on.source = EXTERNAL_OSC;
        event_on.dur = action->duration;
        sg->NoteOn(event_on);
      }
      if (action->duration) {
        auto note_off =
            std::make_unique<AudioActionItem>(AudioAction::MIDI_NOTE_OFF);
        note_off->soundgen_num = action->soundgen_num;
        note_off->notes = action->notes;
        note_off->velocity = action->velocity;
        note_off->start_at = timing_info.midi_tick + action->duration;
        delayed_action_items_.push_back(std::move(note_off));
      }
    }
  } else if (action->type == AudioAction::MIDI_NOTE_OFF_DELAYED) {
    auto note_off =
        std::make_unique<AudioActionItem>(AudioAction::MIDI_NOTE_OFF);
    note_off->soundgen_num = action->soundgen_num;
    note_off->notes = action->notes;
    note_off->velocity = 0;
    note_off->start_at = timing_info.midi_tick + action->note_start_time;
    delayed_action_items_.push_back(std::move(note_off));
  } else if (action->type == AudioAction::MIDI_NOTE_OFF) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      auto &sg = sound_generators_[action->soundgen_num];
      for (auto midinum : action->notes) {
        midi_event event_off = new_midi_event(MIDI_OFF, midinum, 0);
        if (event_off.data1 == 0) {
          sg->AllNotesOff();
        } else {
          sg->NoteOff(event_off);
        }
      }
    }
  } else if (action->type == AudioAction::SOLO) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      soloed_sound_generator_idz.push_back(action->soundgen_num);
      // Fade out all generators not in the solo group
      for (int i = 0; i < sound_generators_idx_; i++) {
        bool is_soloed = std::find(soloed_sound_generator_idz.begin(),
                                   soloed_sound_generator_idz.end(),
                                   i) != soloed_sound_generator_idz.end();
        solo_gain_target_[i] = is_soloed ? 1.0 : 0.0;
      }
    }
  } else if (action->type == AudioAction::UNSOLO) {
    soloed_sound_generator_idz.clear();
    // Fade all generators back in
    for (int i = 0; i < sound_generators_idx_; i++) solo_gain_target_[i] = 1.0;
  } else if (action->type == AudioAction::STOP) {
    if (IsValidSoundgenNum(action->soundgen_num)) {
      auto &sg = sound_generators_[action->soundgen_num];
      // sg->AllNotesOff();
      sg->Stop();
    }
  } else if (action->type == AudioAction::MIXER_UPDATE) {
    double param_val = std::stod(action->param_val);
    if (action->mixer_fx_id != -1) {
      fx_[action->mixer_fx_id]->SetParam(action->param_name, param_val);
    } else if (action->is_xfader) {
      xfader_.Set(action->param_name, param_val);
    }
  } else if (action->type == AudioAction::MIXER_FX_UPDATE) {
    for (const auto &soundgen_num : action->group_of_soundgens) {
      if (IsValidSoundgenNum(soundgen_num)) {
        auto &sg = sound_generators_[soundgen_num];
        if (sg) {
          sg->SetFxSend(action->mixer_fx_id, action->fx_intensity);
        }
      }
    }
  } else if (action->type == AudioAction::MIXER_XFADE_ACTION) {
    if (action->xfade_direction == 0) {
      xfader_.Set("xpos", -1);
    }
    if (action->xfade_direction == 1) {
      xfader_.Set("xpos", 1);
    }
  } else if (action->type == AudioAction::MIXER_XFADE_ASSIGN) {
    for (const auto &soundgen_num : action->group_of_soundgens) {
      if (IsValidSoundgenNum(soundgen_num)) {
        std::cout << "Adding soundgen num:" << soundgen_num << std::endl;
        xfader_.Assign(action->xfade_channel, soundgen_num);
      }
    }
  } else if (action->type == AudioAction::MIXER_XFADE_CLEAR) {
    xfader_.Clear();
  } else if (action->type == AudioAction::UPDATE) {
    if (IsValidSoundgenNum(action->mixer_soundgen_idx)) {
      double param_val = std::stod(action->param_val);

      auto &sg = sound_generators_[action->mixer_soundgen_idx];
      if (!sg) {
        std::cerr << "WHOE NELLY! Naw SG! bailing out!\n";
        return;
      }
      if (action->param_name == "volume" || action->param_name == "vol")
        sg->SetVolume(param_val);
      else if (action->param_name == "pan")
        sg->SetPan(param_val);
      else {
        // first check if we're setting an FX param
        if (action->fx_id != -1) {
          int fx_num = action->fx_id;
          if (IsValidFx(action->mixer_soundgen_idx, fx_num)) {
            // Shared lock to safely access effects array (reading to call
            // SetParam)
            std::shared_lock<std::shared_mutex> lock(sg->effects_mutex_);
            auto f = sg->effects_[fx_num];
            if (action->param_name == "active")
              f->enabled_ = param_val;
            else
              f->SetParam(action->param_name, param_val);
          } else {
            // if is GranularLooper - which re-uses the fx_id slot.
            if (sg->type == LOOPER_TYPE) {
              sg->SetSubParam(fx_num, action->param_name, param_val);
            }
          }
        } else  // must be a SoundGenerator param
        {
          sg->SetParam(action->param_name, param_val);
        }
      }
    }
  } else if (action->type == AudioAction ::INFO) {
    if (IsValidSoundgenNum(action->mixer_soundgen_idx)) {
      auto &sg = sound_generators_[action->mixer_soundgen_idx];
      repl_queue.push(sg->Info());
    }
  } else if (action->type == AudioAction ::SAVE_PRESET ||
             action->type == AudioAction ::RAND_PRESET ||
             action->type == AudioAction ::LIST_PRESETS) {
    interpreter_sound_cmds::ParseSynthCmd(action->args);
  } else if (action->type == AudioAction::LOAD_PRESET) {
    interpreter_sound_cmds::SynthLoadPreset(
        action->args[0], action->preset_name, action->preset);
  } else if (action->type == AudioAction::SAVE_DRUM_PART ||
             action->type == AudioAction::LOAD_DRUM_PART) {
    interpreter_sound_cmds::DrumPartCmd(
        action->args[0], action->preset_name, action->part_name,
        action->type == AudioAction::SAVE_DRUM_PART);
  } else if (action->type == AudioAction::RAND) {
    sound_generators_[action->mixer_soundgen_idx]->Randomize();
  } else if (action->type == AudioAction::PREVIEW) {
    PreviewAudio(std::move(action));
  }
}

void Mixer::ResetMidiRecording() {
  recording_buffer_.assign(record_bars_, MultiEventMidiPattern{});
  pending_note_ons_.clear();
}

void Mixer::SetRecordBars(int bars) {
  if (bars < 1) bars = 1;
  if (bars > 32) bars = 32;
  record_bars_ = bars;
  ResetMidiRecording();
  repl_queue.push("MIDI record bars: " + std::to_string(bars) + "\n");
}

void Mixer::AssignSoundGeneratorToMidiController(int soundgen_id) {
  if (IsValidSoundgenNum(soundgen_id)) {
    std::cout << "MIDI Assign - " << soundgen_id << std::endl;
    midi_target = soundgen_id;
    // midi_targets.push_back(soundgen_id);
  }
}

void Mixer::RecordMidiToggle() {
  if (midi_recording) {
    if (IsValidSoundgenNum(midi_target))
      sound_generators_[midi_target]->AllNotesOff();
    pending_note_ons_.clear();
    repl_queue.push("MIDI REC off\n");
  } else {
    repl_queue.push("MIDI REC on\n");
  }
  midi_recording = !midi_recording;
}

void Mixer::MidiLoopToggle() {
  midi_loop_ = !midi_loop_;
  if (!midi_loop_ && IsValidSoundgenNum(midi_target))
    sound_generators_[midi_target]->AllNotesOff();
  repl_queue.push(midi_loop_ ? "MIDI LOOP on\n" : "MIDI LOOP off\n");
}

void Mixer::MidiStop() {
  midi_recording = false;
  midi_loop_ = false;
  pending_note_ons_.clear();
  if (IsValidSoundgenNum(midi_target))
    sound_generators_[midi_target]->AllNotesOff();
  repl_queue.push("MIDI stop\n");
}

void Mixer::PrintMidiToggle() {
  midi_print = 1 - midi_print;
}

void Mixer::HandleMidiControlMessage(int data1, int data2) {
  if (midi_mapped_controls_.count(data1)) {
    auto param = midi_mapped_controls_[data1];
    const auto &[lo, hi] = GetBoundaries(param);
    auto val = scaleybum(0, 127, lo, hi, data2);
    std::stringstream ss;
    ss << "set " << param << " " << val;
    eval_command_queue.try_push(ss.str());
  }
}

void Mixer::CheckForExternalMidiEvents() {
  if (!have_midi_controller) return;

  int total_ticks = record_bars_ * PPBAR;
  int pos =
      timing_info.midi_tick % total_ticks;  // position in multi-bar buffer
  int cur_bar = pos / PPBAR;
  int tic = pos % PPBAR;

  PmEvent msg[32];
  if (Pm_Poll(midi_stream)) {
    int cnt = Pm_Read(midi_stream, msg, 32);
    for (int i = 0; i < cnt; i++) {
      int status = Pm_MessageStatus(msg[i].message);
      int data1 = Pm_MessageData1(msg[i].message);
      int data2 = Pm_MessageData2(msg[i].message);

      midi_event ev = new_midi_event(status, data1, data2);
      ev.original_tick = timing_info.midi_tick;

      if (midi_recording) {
        if (status == MIDI_ON) {
          pending_note_ons_[data1] = pos;  // absolute position in multi-bar
          recording_buffer_[cur_bar][tic].push_back(ev);
        } else if (status == MIDI_OFF) {
          auto it = pending_note_ons_.find(data1);
          if (it != pending_note_ons_.end()) {
            int on_pos = it->second;
            int on_bar = on_pos / PPBAR;
            int on_tic = on_pos % PPBAR;
            int dur = (pos - on_pos + total_ticks) % total_ticks;
            if (dur == 0) dur = 1;
            for (auto &recorded_ev : recording_buffer_[on_bar][on_tic]) {
              if (recorded_ev.event_type == MIDI_ON &&
                  recorded_ev.data1 == data1) {
                recorded_ev.dur = dur;
                break;
              }
            }
            pending_note_ons_.erase(it);
          }
          // MIDI_OFF not stored; reconstructed from dur on playback
        }
      }

      if (midi_print) {
        std::cout << (timing_info.sixteenth_note_tick % 16) << " - MIDI -- "
                  << status << " " << data1 << " " << data2 << std::endl;
      }

      // Live routing to synth (always, regardless of record state)
      if (IsValidSoundgenNum(midi_target)) {
        if (status == MIDI_ON || status == MIDI_OFF) {
          sound_generators_[midi_target]->ParseMidiEvent(ev, timing_info);
        } else if (status == MIDI_CONTROL) {
          HandleMidiControlMessage(data1, data2);
        }
      }
    }
  }
}

void Mixer::PlaybackMidiLoopTick() {
  if (!IsValidSoundgenNum(midi_target)) return;
  int pos = timing_info.midi_tick % (record_bars_ * PPBAR);
  int cur_bar = pos / PPBAR;
  int tic = pos % PPBAR;
  auto &sg = sound_generators_[midi_target];
  for (const auto &ev : recording_buffer_[cur_bar][tic]) {
    if (ev.event_type == MIDI_ON || ev.event_type == MIDI_OFF) {
      sg->ParseMidiEvent(ev, timing_info);
      if (ev.event_type == MIDI_ON && ev.dur > 0) {
        auto note_off = new_midi_event(MIDI_OFF, ev.data1, 0);
        auto action =
            std::make_unique<AudioActionItem>(AudioAction::RECORDED_MIDI_EVENT);
        action->event = note_off;
        action->mixer_soundgen_idx = midi_target;
        action->start_at = timing_info.midi_tick + ev.dur;
        delayed_action_items_.push_back(std::move(action));
      }
    }
  }
}

void Mixer::EmitTimingDisplay() {
  int step = timing_info.sixteenth_note_tick % 16;        // 0..15 within bar
  int beat = (timing_info.midi_tick % PPBAR) / PPQN + 1;  // 1..4
  int sub = (step % 4) + 1;  // 1..4 sixteenth within beat
  int cur_bar = (timing_info.midi_tick % (record_bars_ * PPBAR)) / PPBAR + 1;

  // ── Measure visible widths (terminal columns, not UTF-8 bytes) ──────────
  // Total line width is fixed at 80 cols:
  //   left colon-fill  |  timing (right-aligned)  |  "  ::" = 4 cols
  // left_fill = 80 - 4 - timing_visible
  constexpr int kLineWidth = 80;
  constexpr int kRightWidth = 4;  // "  ::"

  char bpm_buf[16];
  snprintf(bpm_buf, sizeof(bpm_buf), "%.1f",
           static_cast<double>(timing_info.bpm));

  // Position string (all ASCII, so size() == visible cols)
  std::string pos_str;
  if (record_bars_ > 1)
    pos_str =
        std::to_string(cur_bar) + "/" + std::to_string(record_bars_) + " ";
  pos_str += std::to_string(beat) + "." + std::to_string(sub);

  // "140.0 BPM  2.3  [################]" — block chars are 1 terminal col each
  int timing_vis = (int)strlen(bpm_buf) + 4  // "140.0 BPM"
                   + 2                       // "  "
                   + (int)pos_str.size()     // "2.3" or "2/4 3.2"
                   + 3 + 16 + 1;             // "  [" + 16 steps + "]"
  // ⏺ (U+23FA) and ▶ (U+25B6) are each 3 UTF-8 bytes but 1 terminal col
  if (midi_recording) timing_vis += 5;  // " ⏺REC"  → 1+1+3 = 5 cols
  if (midi_loop_) timing_vis += 6;      // " ▶LOOP" → 1+1+4 = 6 cols

  int left_fill = kLineWidth - kRightWidth - timing_vis;
  if (left_fill < 1) left_fill = 1;

  // term_rows_ is set once at startup by cmdloop; we just read it here.
  int status_row = (term_rows_ > 1) ? term_rows_ : 24;

  // ── Build output ─────────────────────────────────────────────────────────
  std::stringstream ss;
  ss << "\033[s"                         // save cursor position only
     << "\033[" << status_row << ";1H";  // move to fixed bottom row

  // Left: colon-fill up to the BPM field (dim green)
  ss << COOL_COLOR_GREEN;
  for (int i = 0; i < left_fill; i++) ss << ':';
  ss << ANSI_COLOR_RESET;

  // BPM (blue)
  ss << COOL_COLOR_BLUE << bpm_buf << " BPM" << ANSI_COLOR_RESET << "  ";

  // Position (cyan)
  ss << ANSI_COLOR_CYAN << pos_str << ANSI_COLOR_RESET << "  ";

  // 16-step bar progress
  ss << "[";
  for (int i = 0; i < 16; i++) {
    if (i < step)
      ss << COOL_COLOR_GREEN << "█";
    else if (i == step)
      ss << COOL_COLOR_YELLOW << "▌";
    else
      ss << ANSI_COLOR_WHITE << "░";
  }
  ss << ANSI_COLOR_RESET << "]";

  // Transport status
  if (midi_recording) ss << ANSI_COLOR_RED << " ⏺REC" << ANSI_COLOR_RESET;
  if (midi_loop_) ss << COOL_COLOR_GREEN << " ▶LOOP" << ANSI_COLOR_RESET;

  // Right: "  ::" then clear to EOL and restore cursor (position only)
  ss << "  " << COOL_COLOR_GREEN << "::" << ANSI_COLOR_RESET << "\033[K\033[u";

  repl_queue.push(ss.str());
}

void Mixer::QuantizeMidiRecording(int subdivisions) {
  int grid = PPBAR / subdivisions;
  if (grid <= 0) return;
  for (auto &bar : recording_buffer_) {
    MultiEventMidiPattern quantized{};
    for (int i = 0; i < PPBAR; i++) {
      for (auto &ev : bar[i]) {
        int q_tick = ((i + grid / 2) / grid) * grid % PPBAR;
        quantized[q_tick].push_back(ev);
      }
    }
    bar = quantized;
  }
}

void Mixer::CheckForAudioActionQueueMessages() {
  while (auto opt_action = audio_queue.try_pop()) {
    auto &action = opt_action.value();
    if (action->delayed_by > 0) {
      action->start_at = timing_info.midi_tick + action->delayed_by;
      action->delayed_by = 0;
      delayed_action_items_.push_back(std::move(action));
    } else {
      ProcessActionMessage(std::move(action));
    }
  }
}

void Mixer::AddFileToMonitor(std::string filepath) {
  // Check if already monitoring this file
  for (const auto &f : file_monitors) {
    if (f.function_file_filepath == filepath) {
      return;  // Already monitoring
    }
  }
  file_monitors.push_back(
      file_monitor{.function_file_filepath = filepath,
                   .function_file_filepath_last_write_time = {},
                   .initialized = false});
}

void Mixer::CheckForDelayedEvents() {
  std::vector<std::unique_ptr<AudioActionItem>> delayed_buffer;
  auto dit = delayed_action_items_.begin();
  while (dit != delayed_action_items_.end()) {
    if ((*dit)->start_at == (timing_info.midi_tick % PPBAR)) {
      if ((*dit)->event.event_type == MIDI_CONTROL) {
        HandleMidiControlMessage((*dit)->event.data1, (*dit)->event.data2);
      } else if (IsValidSoundgenNum((*dit)->mixer_soundgen_idx)) {
        auto &sg = sound_generators_[(*dit)->mixer_soundgen_idx];
        sg->ParseMidiEvent((*dit)->event, timing_info);
        if ((*dit)->event.event_type == MIDI_ON && (*dit)->event.dur > 0) {
          auto note_off = new_midi_event(MIDI_OFF, (*dit)->event.data1, 0);
          auto action = std::make_unique<AudioActionItem>(
              AudioAction::RECORDED_MIDI_EVENT);
          action->event = note_off;
          action->mixer_soundgen_idx = (*dit)->mixer_soundgen_idx;
          action->start_at = timing_info.midi_tick + (*dit)->event.dur;
          delayed_buffer.push_back(std::move(action));
        }
      }
      dit = delayed_action_items_.erase(dit);
    } else if ((*dit)->start_at == timing_info.midi_tick) {
      if ((*dit)->type == RECORDED_MIDI_EVENT) {
        if (IsValidSoundgenNum((*dit)->mixer_soundgen_idx)) {
          auto &sg = sound_generators_[(*dit)->mixer_soundgen_idx];
          sg->ParseMidiEvent((*dit)->event, timing_info);
        }
      } else {
        ProcessActionMessage(std::move(*dit));
      }
      // `erase()` invalidates the iterator, use returned iterator
      dit = delayed_action_items_.erase(dit);
    } else {
      ++dit;
    }
  }
  for (auto &d : delayed_buffer) {
    delayed_action_items_.push_back(std::move(d));
  }
}

void Mixer::PrintRecordingBuffer() {
  for (int b = 0; b < record_bars_; b++) {
    std::cout << "--- bar " << (b + 1) << " ---\n";
    PrintMultiMidi(recording_buffer_[b]);
  }
}
void Mixer::AddMidiMapping(int id, std::string param) {
  midi_mapped_controls_[id] = param;
}

void Mixer::ResetMidiMappings() {
  midi_mapped_controls_.clear();
}
void Mixer::PrintMidiMappings() {
  for (const auto &[id, param] : midi_mapped_controls_) {
    std::cout << "ID:" << id << " // Param:" << param << std::endl;
  }
}
