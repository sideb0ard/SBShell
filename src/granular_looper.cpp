#include "granular_looper.h"

#include <audioutils.h>
#include <libgen.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>

#include <iostream>

#include "defjams.h"
#include "mixer.h"

namespace SBAudio {

namespace {

const std::array<std::string, 3> kLoopModeNames = {"LOOP", "STATIC", "SMUDGE"};

void ClearPattern(std::array<int, 16> &pattern) {
  for (int i = 0; i < 16; ++i) {
    pattern[i] = 0;
  }
}
void StutterPattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  int idx = 0;
  for (int i = 0; i < 16; ++i) {
    if (rand() % 100 > 70) idx++;
    pattern[i] = idx;
  }
}

void ScramblePattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  for (int i = 0; i < 16; ++i) {
    pattern[i] = i;
    if (i % 4 > 0) {
      pattern[i] = rand() % 16;
    }
    if (rand() % 100 > 90) {
      pattern[i] = 0;
    }
  }
  pattern[15] = 0;
}

void SpeedulatePattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  // Randomly pick half-speed or double-speed per bar
  int mode = rand() % 3;

  if (mode == 0) {
    // Half-speed: each slice plays twice, covers first 8 slices
    for (int i = 0; i < 16; ++i) {
      pattern[i] = i / 2;
    }
  } else if (mode == 1) {
    // Double-speed: skip every other slice, then wrap around
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i * 2) % 16;
    }
  } else {
    // Mixed: half-speed first half, double-speed second half
    for (int i = 0; i < 8; ++i) {
      pattern[i] = i / 2;
    }
    for (int i = 8; i < 16; ++i) {
      pattern[i] = ((i - 8) * 2) % 16;
    }
  }
}

void GatePattern(std::array<int, 16> &pattern) {
  // Pick a rhythmic gate style
  int mode = rand() % 4;

  if (mode == 0) {
    // Off-beat gate: silence every other step
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i % 2 == 0) ? 1 : 0;
    }
  } else if (mode == 1) {
    // Sparse: only let through every 4th step
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i % 4 == 0) ? 1 : 0;
    }
  } else if (mode == 2) {
    // Syncopated: a funky rhythmic pattern
    const int p[16] = {1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0};
    for (int i = 0; i < 16; ++i) {
      pattern[i] = p[i];
    }
  } else {
    // Random gate: ~40% chance each step is silenced
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (rand() % 100 > 40) ? 1 : 0;
    }
    // Always keep the downbeat
    pattern[0] = 1;
  }
}

void SlowdownPattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  // Tape stop: normal playback that decelerates into a stutter
  int mode = rand() % 3;

  if (mode == 0) {
    // Decelerate in the last quarter
    for (int i = 0; i < 12; ++i) {
      pattern[i] = i;
    }
    pattern[12] = 12;
    pattern[13] = 12;
    pattern[14] = 13;
    pattern[15] = 13;
  } else if (mode == 1) {
    // Decelerate in the second half
    for (int i = 0; i < 8; ++i) {
      pattern[i] = i;
    }
    for (int i = 8; i < 16; ++i) {
      pattern[i] = 8 + (i - 8) / 2;
    }
  } else {
    // Full deceleration: progressively repeats more
    pattern[0] = 0;
    pattern[1] = 1;
    pattern[2] = 2;
    pattern[3] = 3;
    pattern[4] = 4;
    pattern[5] = 5;
    pattern[6] = 6;
    pattern[7] = 6;
    pattern[8] = 7;
    pattern[9] = 7;
    pattern[10] = 7;
    pattern[11] = 8;
    pattern[12] = 8;
    pattern[13] = 8;
    pattern[14] = 8;
    pattern[15] = 8;
  }
}

void RepeatPattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  // Beat repeat: pick a slice and repeat it for N steps
  int mode = rand() % 4;

  if (mode == 0) {
    // 4-step repeat starting at a random beat boundary
    int repeat_slice = (rand() % 4) * 4;
    for (int i = 0; i < 16; ++i) {
      pattern[i] = i;
    }
    for (int i = repeat_slice; i < repeat_slice + 4 && i < 16; ++i) {
      pattern[i] = repeat_slice;
    }
  } else if (mode == 1) {
    // 8-step repeat in the second half
    for (int i = 0; i < 8; ++i) {
      pattern[i] = i;
    }
    int repeat_slice = rand() % 8;
    for (int i = 8; i < 16; ++i) {
      pattern[i] = repeat_slice;
    }
  } else if (mode == 2) {
    // Rapid 2-step repeats scattered through the bar
    for (int i = 0; i < 16; ++i) {
      pattern[i] = i;
    }
    int num_repeats = 2 + rand() % 2;
    for (int r = 0; r < num_repeats; ++r) {
      int pos = (rand() % 8) * 2;
      pattern[pos + 1] = pattern[pos];
    }
  } else {
    // Escalating repeat: 1x, 2x, 4x repeat of same slice
    int slice = rand() % 4;
    pattern[0] = 0;
    pattern[1] = 1;
    pattern[2] = 2;
    pattern[3] = 3;
    pattern[4] = slice;
    pattern[5] = slice;
    pattern[6] = 6;
    pattern[7] = 7;
    pattern[8] = slice;
    pattern[9] = slice;
    pattern[10] = slice;
    pattern[11] = slice;
    pattern[12] = 12;
    pattern[13] = 13;
    pattern[14] = 14;
    pattern[15] = 15;
  }
}

void StrobePattern(std::array<int, 16> &pattern) {
  ClearPattern(pattern);

  // Strobe: alternate between an anchor slice and the normal sequence
  int mode = rand() % 3;

  if (mode == 0) {
    // Classic strobe: anchor on slice 0
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i % 2 == 0) ? 0 : i;
    }
  } else if (mode == 1) {
    // Strobe with random anchor
    int anchor = rand() % 16;
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i % 2 == 0) ? anchor : i;
    }
  } else {
    // Double strobe: two anchors alternating
    int anchor_a = rand() % 8;
    int anchor_b = 8 + rand() % 8;
    for (int i = 0; i < 16; ++i) {
      if (i % 4 == 0)
        pattern[i] = anchor_a;
      else if (i % 4 == 2)
        pattern[i] = anchor_b;
      else
        pattern[i] = i;
    }
  }
}

void ClearPitchPattern(std::array<double, 16> &pattern) {
  for (int i = 0; i < 16; ++i) {
    pattern[i] = 1.0;
  }
}

void PitchRampPattern(std::array<double, 16> &pattern) {
  ClearPitchPattern(pattern);

  int mode = rand() % 3;

  if (mode == 0) {
    // Ramp up: pitch rises across the bar (1.0 to 2.0)
    for (int i = 0; i < 16; ++i) {
      pattern[i] = 1.0 + (i / 15.0);
    }
  } else if (mode == 1) {
    // Ramp down: pitch falls across the bar (1.0 to 0.5)
    for (int i = 0; i < 16; ++i) {
      pattern[i] = 1.0 - (i / 15.0) * 0.5;
    }
  } else {
    // V-shape: pitch drops to half then rises back up
    for (int i = 0; i < 8; ++i) {
      pattern[i] = 1.0 - (i / 7.0) * 0.5;
    }
    for (int i = 8; i < 16; ++i) {
      pattern[i] = 0.5 + ((i - 8) / 7.0) * 0.5;
    }
  }
}

void OctaveJumpPattern(std::array<double, 16> &pattern) {
  ClearPitchPattern(pattern);

  const double octaves[] = {0.5, 1.0, 2.0};

  int mode = rand() % 3;

  if (mode == 0) {
    // Random octave per step
    for (int i = 0; i < 16; ++i) {
      pattern[i] = octaves[rand() % 3];
    }
    // Keep downbeat at normal pitch
    pattern[0] = 1.0;
  } else if (mode == 1) {
    // Octave up on off-beats
    for (int i = 0; i < 16; ++i) {
      pattern[i] = (i % 2 == 0) ? 1.0 : 2.0;
    }
  } else {
    // Alternating octave down / normal per beat
    for (int i = 0; i < 16; ++i) {
      pattern[i] = ((i / 4) % 2 == 0) ? 1.0 : 0.5;
    }
  }
}

void PitchStaircasePattern(std::array<double, 16> &pattern) {
  ClearPitchPattern(pattern);

  // Semitone ratio
  const double semitone = 1.05946309436;

  int mode = rand() % 3;

  if (mode == 0) {
    // Step up a semitone every beat (every 4 sixteenths)
    for (int i = 0; i < 16; ++i) {
      int step = i / 4;
      double ratio = 1.0;
      for (int s = 0; s < step; ++s) ratio *= semitone;
      pattern[i] = ratio;
    }
  } else if (mode == 1) {
    // Step up a semitone every sixteenth
    for (int i = 0; i < 16; ++i) {
      double ratio = 1.0;
      for (int s = 0; s < i; ++s) ratio *= semitone;
      pattern[i] = ratio;
    }
  } else {
    // Step down a semitone every beat
    for (int i = 0; i < 16; ++i) {
      int step = i / 4;
      double ratio = 1.0;
      for (int s = 0; s < step; ++s) ratio /= semitone;
      pattern[i] = ratio;
    }
  }
}

}  // namespace

void GranularLooper::Reset() {
  file_buffer_->audio_buffer_read_idx_ = 0;
  active_grain_ = grain_a_.get();
  incoming_grain_ = grain_b_.get();
  reverse_mode_ = false;
  SetGrainDensity(15);
  eg_.m_reset_to_zero = true;
  eg_.SetAttackTimeMsec(10);  // Slightly longer attack to avoid clicks
  eg_.SetDecayTimeMsec(10);
  eg_.SetSustainLevel(1);
  eg_.SetReleaseTimeMsec(80);
  eg_.SetRampMode(false);
  eg_.Update();
  eg_.StartEg();

  degrade_by_ = 0;

  active = true;
  started_ = false;
  stop_pending_ = false;
}

GranularLooper::GranularLooper() {
  grain_type_ = SoundGrainType::Signal;
  active_grain_ = nullptr;
  incoming_grain_ = nullptr;
}

GranularLooper::GranularLooper(std::string filename, unsigned int loop_mode)
    : grain_a_{std::make_unique<SoundGrainSample>()},
      grain_b_{std::make_unique<SoundGrainSample>()},
      grain_type_{SoundGrainType::Sample},
      file_buffer_{std::make_unique<FileBuffer>(filename)},
      active_grain_{grain_a_.get()},
      incoming_grain_{grain_b_.get()} {
  type = LOOPER_TYPE;
  SetLoopMode(loop_mode);
  Reset();
}

GranularLooper::~GranularLooper() {
  // TODO delete file
}

void GranularLooper::AddBuffer(std::unique_ptr<FileBuffer> fb) {
  file_buffer_ = std::move(fb);
}

void GranularLooper::EventNotify(broadcast_event event,
                                 mixer_timing_info tinfo) {
  (void)event;

  if (!started_ && tinfo.is_start_of_loop) {
    LaunchGrain(active_grain_, tinfo);
    eg_.StartEg();
    started_ = true;
  }
  if (!started_) return;

  const std::vector<double> *audio_buffer = file_buffer_->GetAudioBuffer();

  if (tinfo.is_midi_tick) {
    file_buffer_->cur_midi_idx_ =
        fmodf(file_buffer_->cur_midi_idx_ + file_buffer_->incr_speed_,
              PPBAR * file_buffer_->loop_len_);
    if (file_buffer_->loop_mode_ == LoopMode::loop_mode) {
      double decimal_percent_of_loop =
          file_buffer_->cur_midi_idx_ / (PPBAR * file_buffer_->loop_len_);
      double new_read_idx = decimal_percent_of_loop * audio_buffer->size();
      if (reverse_mode_)
        new_read_idx = (audio_buffer->size() - 1) - new_read_idx;

      // size_of_sixteenth_ is scaled by loop_len_, so recover the 1-bar
      // sixteenth size for ploop region bounds — otherwise a 2-bar loop wraps
      // at half the buffer instead of the full buffer.
      double one_bar_sixteenth =
          file_buffer_->size_of_sixteenth_ * file_buffer_->loop_len_;
      new_read_idx = fmodf(
          (fmodf(new_read_idx, one_bar_sixteenth * file_buffer_->plooplen_) +
           file_buffer_->poffset_ * one_bar_sixteenth),
          audio_buffer->size());

      // this ensures new_read_idx is even
      if (file_buffer_->num_channels_ == 2)
        new_read_idx -= ((int)new_read_idx & 1);

      if (new_read_idx < 0 || new_read_idx > audio_buffer->size() - 1) {
        new_read_idx = 0;
        std::cout << "OH YA:" << new_read_idx
                  << " bufflen:" << (audio_buffer->size() - 1) << std::endl;
      }

      file_buffer_->audio_buffer_read_idx_ = new_read_idx;

      int read_idx = file_buffer_->audio_buffer_read_idx_.load();
      int rel_pos_within_a_sixteenth =
          fmod(static_cast<double>(read_idx), file_buffer_->size_of_sixteenth_);

      if (file_buffer_->stutter_mode_ || file_buffer_->scramble_mode_ ||
          file_buffer_->speedulate_mode_ || file_buffer_->slowdown_mode_ ||
          file_buffer_->repeat_mode_ || file_buffer_->strobe_mode_) {
        int target_slice =
            file_buffer_->scrambled_pattern_[file_buffer_->cur_sixteenth_];
        int normal_slice = file_buffer_->cur_sixteenth_;

        // If FX remapped to a different slice, snap to its start
        // for a punchier transient. Otherwise keep the sub-position.
        int offset =
            (target_slice != normal_slice) ? 0 : rel_pos_within_a_sixteenth;

        file_buffer_->audio_buffer_read_idx_ = static_cast<int>(
            fmodf((target_slice * file_buffer_->size_of_sixteenth_) + offset,
                  audio_buffer->size()));
      }
    }
  }

  if (tinfo.is_end_of_loop) {
    if (stop_count_pending_) {
      stop_countr_++;
      if (stop_countr_ >= stop_len_) {
        Stop();
        stop_count_pending_ = false;
      }
    }
  }
  if (tinfo.is_start_of_loop) {
    if (file_buffer_->scramble_pending_) {
      file_buffer_->scramble_mode_ = true;
      file_buffer_->scramble_pending_ = false;
    } else
      file_buffer_->scramble_mode_ = false;

    if (file_buffer_->stutter_pending_) {
      file_buffer_->stutter_mode_ = true;
      file_buffer_->stutter_pending_ = false;
    } else
      file_buffer_->stutter_mode_ = false;

    if (file_buffer_->speedulate_pending_) {
      file_buffer_->speedulate_mode_ = true;
      file_buffer_->speedulate_pending_ = false;
    } else
      file_buffer_->speedulate_mode_ = false;

    if (file_buffer_->gate_pending_) {
      file_buffer_->gate_mode_ = true;
      file_buffer_->gate_pending_ = false;
    } else
      file_buffer_->gate_mode_ = false;

    if (file_buffer_->slowdown_pending_) {
      file_buffer_->slowdown_mode_ = true;
      file_buffer_->slowdown_pending_ = false;
    } else
      file_buffer_->slowdown_mode_ = false;

    if (file_buffer_->repeat_pending_) {
      file_buffer_->repeat_mode_ = true;
      file_buffer_->repeat_pending_ = false;
    } else
      file_buffer_->repeat_mode_ = false;

    if (file_buffer_->strobe_pending_) {
      file_buffer_->strobe_mode_ = true;
      file_buffer_->strobe_pending_ = false;
    } else
      file_buffer_->strobe_mode_ = false;

    if (file_buffer_->pitch_ramp_pending_) {
      file_buffer_->pitch_ramp_mode_ = true;
      file_buffer_->pitch_ramp_pending_ = false;
    } else
      file_buffer_->pitch_ramp_mode_ = false;

    if (file_buffer_->octave_jump_pending_) {
      file_buffer_->octave_jump_mode_ = true;
      file_buffer_->octave_jump_pending_ = false;
    } else
      file_buffer_->octave_jump_mode_ = false;

    if (file_buffer_->pitch_staircase_pending_) {
      file_buffer_->pitch_staircase_mode_ = true;
      file_buffer_->pitch_staircase_pending_ = false;
    } else
      file_buffer_->pitch_staircase_mode_ = false;

    if (!file_buffer_->pitch_ramp_mode_ && !file_buffer_->octave_jump_mode_ &&
        !file_buffer_->pitch_staircase_mode_) {
      ClearPitchPattern(file_buffer_->pitch_pattern_);
    }

    if (reverse_pending_) {
      reverse_mode_ = true;
      reverse_pending_ = false;
    } else
      reverse_mode_ = false;
  }

  if (tinfo.is_sixteenth) {
    file_buffer_->cur_sixteenth_ = tinfo.sixteenth_note_tick % 16;
  }
}

// for debugging only
int launch_count = 0;
int samp_diff = 0;
int midi_diff = 0;

void GranularLooper::LaunchGrain(SoundGrain *grain, mixer_timing_info tinfo) {
  int duration_frames = grain_duration_frames_;
  if (quasi_grain_fudge_ != 0)
    duration_frames += rand() % (int)(quasi_grain_fudge_ * 44.1);

  std::unique_ptr<FileBuffer> &file_buffer = file_buffer_;
  std::vector<double> *audio_buffer = file_buffer->GetAudioBuffer();

  int grain_idx = file_buffer->audio_buffer_read_idx_;
  if (granular_spray_frames_ > 0)
    grain_idx += (rand() % granular_spray_frames_) % audio_buffer->size();

  SoundGrainParams params = {
      .grain_type = SoundGrainType::Sample,
      .dur_frames = duration_frames,
      .starting_idx = grain_idx,
      .reverse_mode = reverse_mode_,
      .num_channels = file_buffer->num_channels_,
      .degrade_by = degrade_by_,
      .pitch_ratio = file_buffer->pitch_ratio_.load() *
                     file_buffer->pitch_pattern_[file_buffer->cur_sixteenth_],
      .audio_buffer = audio_buffer,
  };

  grain->Initialize(params);

  xfade_time_in_frames_ = duration_frames / 100. * 20;
  grain_spacing_frames_ = duration_frames - xfade_time_in_frames_;

  next_grain_launch_sample_time_ = tinfo.cur_sample + grain_spacing_frames_;
  //  if (launch_count < 10) {
  //    std::cout << launch_count << " Mixer Samp#:" << tinfo.cur_sample
  //              << " dur:" << duration_frames << " GRain IDX:" << grain_idx
  //              << " NEXT LAYNC :" << next_grain_launch_sample_time_
  //              << " MIDI:" << tinfo.midi_tick << " %16 " << tinfo.midi_tick %
  //              16
  //              << std::endl;
  //    launch_count++;
  //  }

  start_xfade_at_frame_time_ = next_grain_launch_sample_time_;
  if (started_)
    stop_xfade_at_frame_time_ = tinfo.cur_sample + xfade_time_in_frames_;
}

void GranularLooper::SwitchXFadeGrains() {
  if (active_grain_ == grain_a_.get()) {
    active_grain_ = grain_b_.get();
    incoming_grain_ = grain_a_.get();
  } else {
    active_grain_ = grain_a_.get();
    incoming_grain_ = grain_b_.get();
  }
}

StereoVal GranularLooper::GenNext(mixer_timing_info tinfo) {
  StereoVal val = {0., 0.};
  if (!started_ || !active) {
    return val;
  }

  if (stop_pending_ && eg_.m_state == OFFF) active = false;

  if (tinfo.cur_sample == start_xfade_at_frame_time_) {
    xfader_active_ = true;
  }
  if (tinfo.cur_sample == stop_xfade_at_frame_time_) {
    SwitchXFadeGrains();
    xfader_active_ = false;
    xfader_.Reset(xfade_time_in_frames_);
  }

  if (tinfo.cur_sample >= next_grain_launch_sample_time_) {  // new grain time
    LaunchGrain(incoming_grain_, tinfo);
  }

  StereoVal active_val = active_grain_->Generate();
  StereoVal incoming_val = incoming_grain_->Generate();

  if (xfader_active_) {
    double t = xfader_.Generate();
    double incoming_vol = std::sin(t * M_PI * 0.5);
    double active_vol = std::cos(t * M_PI * 0.5);

    val.left = active_val.left * active_vol + incoming_val.left * incoming_vol;
    val.right =
        active_val.right * active_vol + incoming_val.right * incoming_vol;
  } else {
    val.left = active_val.left;
    val.right = active_val.right;
  }

  eg_.Update();
  double eg_amp = eg_.DoEnvelope(NULL);

  pan = fmin(pan, 1.0);
  pan = fmax(pan, -1.0);
  double pan_left = 0.707;
  double pan_right = 0.707;
  calculate_pan_values(pan, &pan_left, &pan_right);

  val.left = val.left * volume * eg_amp * pan_left;
  val.right = val.right * volume * eg_amp * pan_right;

  if (file_buffer_->gate_mode_) {
    int cur_step = file_buffer_->cur_sixteenth_;
    int gate_open = file_buffer_->gate_pattern_[cur_step];
    int sixteenth_size = file_buffer_->size_of_sixteenth_;
    int read_idx = file_buffer_->audio_buffer_read_idx_.load();
    int pos_in_step = static_cast<int>(fmod(
        static_cast<double>(read_idx), static_cast<double>(sixteenth_size)));

    double gate_amp = 0.0;

    if (gate_open && sixteenth_size > 0) {
      double pos_ratio = static_cast<double>(pos_in_step) / sixteenth_size;

      // Envelope shape: quick attack, sustain, smooth cosine release
      // Attack:  first 5% — fast linear ramp up
      // Sustain: 5% to 60% — full volume
      // Release: 60% to 100% — cosine curve down to silence
      if (pos_ratio < 0.05) {
        gate_amp = pos_ratio / 0.05;
      } else if (pos_ratio < 0.6) {
        gate_amp = 1.0;
      } else {
        // Cosine release: smooth curve from 1.0 to 0.0
        double release_ratio = (pos_ratio - 0.6) / 0.4;
        gate_amp = 0.5 * (1.0 + cos(M_PI * release_ratio));
      }
    }

    val.left *= gate_amp;
    val.right *= gate_amp;
  }

  val = Effector(val);

  return val;
}

std::string GranularLooper::Status() {
  std::stringstream ss;
  if (!active || volume == 0)
    ss << ANSI_COLOR_RESET;
  else
    ss << ANSI_COLOR_RED;
  ss << "SBPlayer // " << file_buffer_->filename_ << "\n";
  ss << "vol:" << volume << " pan:" << pan
     << " pitch:" << file_buffer_->pitch_ratio_ << " idx:"
     << (int)(100. / file_buffer_->GetAudioBuffer()->size() *
              file_buffer_->audio_buffer_read_idx_)
     << " mode:" << kLoopModeNames[file_buffer_->loop_mode_] << "("
     << file_buffer_->loop_mode_ << ")"
     << " len:" << file_buffer_->loop_len_;
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

std::string GranularLooper::Info() {
  const char *INSTRUMENT_COLOR = ANSI_COLOR_RESET;
  if (active) INSTRUMENT_COLOR = ANSI_COLOR_RED;

  std::stringstream ss;
  ss << INSTRUMENT_COLOR << "\nSBPlayer // vol:" << volume << " pan:" << pan
     << " pitch:" << file_buffer_->pitch_ratio_
     << "\ngrain_dur_ms:" << grain_duration_frames_
     << " grains_per_sec:" << grains_per_sec_
     << " quasi_grain_fudge:" << quasi_grain_fudge_
     << " grain_spray_ms:" << granular_spray_frames_ / 44.1
     << "\nattack:" << eg_.m_attack_time_msec
     << " decay:" << eg_.m_decay_time_msec
     << " release:" << eg_.m_release_time_msec
     << " grain_ramp_time:" << grain_ramp_time_ << "\n";

  int idx = 0;
  ss << ANSI_COLOR_WHITE << idx++ << " " << file_buffer_->filename_
     << " speed:" << file_buffer_->incr_speed_
     << " mode:" << kLoopModeNames[file_buffer_->loop_mode_]
     << " poffset:" << file_buffer_->poffset_
     << " plooplen:" << file_buffer_->plooplen_
     << " pinc:" << file_buffer_->pinc_;

  return ss.str();
}

void GranularLooper::Start() {
  Reset();
}

void GranularLooper::Stop() {
  eg_.Release();
  stop_pending_ = true;
}

void GranularLooper::SetGrainDuration(int dur) {
  grains_per_sec_ = 1000. / dur;
  grain_duration_frames_ = (double)SAMPLE_RATE / grains_per_sec_;
}

void GranularLooper::SetGrainDensity(int gps) {
  grains_per_sec_ = gps;
  grain_duration_frames_ = (double)SAMPLE_RATE / grains_per_sec_;
  grain_ramp_time_ = grain_duration_frames_ / 100. * 10;
  xfade_time_in_frames_ = grain_duration_frames_ / 100. * 20;
  xfader_.Reset(xfade_time_in_frames_);
  grain_spacing_frames_ = grain_duration_frames_ - xfade_time_in_frames_;
}

void GranularLooper::SetGranularSpray(int spray_ms) {
  granular_spray_frames_ = spray_ms / 1000 * SAMPLE_RATE;
}

void GranularLooper::SetQuasiGrainFudge(int fudgefactor) {
  quasi_grain_fudge_ = fudgefactor;
}

void GranularLooper::SetPitch(double pitch_ratio) {
  file_buffer_->SetPitch(pitch_ratio);
}

void GranularLooper::SetIncrSpeed(double speed) {
  file_buffer_->incr_speed_ = speed;
}

void GranularLooper::SetReverseMode(bool b) {
  reverse_mode_ = b;
}

void GranularLooper::SetLoopMode(unsigned int m) {
  // volume = 0.2;
  const std::unique_ptr<FileBuffer> &buffer = file_buffer_;
  switch (m) {
    case (0):
      buffer->loop_mode_ = LoopMode::loop_mode;
      quasi_grain_fudge_ = 0;
      granular_spray_frames_ = 0;
      // volume = 1;
      break;
    case (1):
      buffer->loop_mode_ = LoopMode::static_mode;
      quasi_grain_fudge_ = 0;
      granular_spray_frames_ = 0;
      break;
    case (2):
    default:
      buffer->loop_mode_ = LoopMode::smudge_mode;
      quasi_grain_fudge_ = 220;
      granular_spray_frames_ = 441;  // 10ms * (44100/1000)
  }
}
void GranularLooper::SetScramblePending() {
  std::unique_ptr<FileBuffer> &buffer = file_buffer_;
  buffer->scramble_pending_ = true;
  ScramblePattern(buffer->scrambled_pattern_);
}

void GranularLooper::SetStopPending(int loops) {
  stop_count_pending_ = true;
  stop_len_ = loops;
  stop_countr_ = 0;
}

void GranularLooper::SetStutterPending() {
  file_buffer_->stutter_pending_ = true;
  StutterPattern(file_buffer_->scrambled_pattern_);
}
void GranularLooper::SetReversePending() {
  reverse_pending_ = true;
}

void GranularLooper::SetSpeedulatePending() {
  file_buffer_->speedulate_pending_ = true;
  SpeedulatePattern(file_buffer_->scrambled_pattern_);
}

void GranularLooper::SetGatePending() {
  file_buffer_->gate_pending_ = true;
  GatePattern(file_buffer_->gate_pattern_);
}

void GranularLooper::SetSlowdownPending() {
  file_buffer_->slowdown_pending_ = true;
  SlowdownPattern(file_buffer_->scrambled_pattern_);
}

void GranularLooper::SetRepeatPending() {
  file_buffer_->repeat_pending_ = true;
  RepeatPattern(file_buffer_->scrambled_pattern_);
}

void GranularLooper::SetStrobePending() {
  file_buffer_->strobe_pending_ = true;
  StrobePattern(file_buffer_->scrambled_pattern_);
}

void GranularLooper::SetPitchRampPending() {
  file_buffer_->pitch_ramp_pending_ = true;
  PitchRampPattern(file_buffer_->pitch_pattern_);
}

void GranularLooper::SetOctaveJumpPending() {
  file_buffer_->octave_jump_pending_ = true;
  OctaveJumpPattern(file_buffer_->pitch_pattern_);
}

void GranularLooper::SetPitchStaircasePending() {
  file_buffer_->pitch_staircase_pending_ = true;
  PitchStaircasePattern(file_buffer_->pitch_pattern_);
}

void GranularLooper::SetLoopLen(double bars) {
  if (bars != 0) {
    file_buffer_->SetLoopLen(bars);
  }
}

void GranularLooper::SetDegradeBy(int degradation) {
  if (degradation >= 0 && degradation <= 100) degrade_by_ = degradation;
}

void GranularLooper::NoteOn(midi_event ev) {
  started_ = true;
  int sixteenth_pos = ev.data1 % 16;
  file_buffer_->cur_midi_idx_ = sixteenth_pos * PPSIXTEENTH;

  // Snap to the exact start of the sixteenth slice
  double new_read_idx = sixteenth_pos * file_buffer_->size_of_sixteenth_;

  // Ensure stereo alignment
  if (file_buffer_->num_channels_ == 2) new_read_idx -= ((int)new_read_idx & 1);

  file_buffer_->audio_buffer_read_idx_ = static_cast<int>(new_read_idx);
  grain_a_->SetReadIdx(new_read_idx);
  grain_b_->SetReadIdx(new_read_idx);
  next_grain_launch_sample_time_ = 0;
  eg_.StartEg();
}

void GranularLooper::AllNotesOff() {
  started_ = true;
  eg_.NoteOff();
}

void GranularLooper::NoteOff(midi_event ev) {
  (void)ev;
  started_ = true;
  eg_.NoteOff();
}

void GranularLooper::SetPidx(int val) {
  file_buffer_->poffset_ = abs(val - file_buffer_->cur_sixteenth_) % 16;
}

void GranularLooper::SetPOffset(int p) {
  if (p >= 0 && p <= 15) {
    file_buffer_->poffset_ = p;
  }
}

void GranularLooper::SetPlooplen(int plooplen) {
  if (plooplen > 0 && plooplen <= 16) {
    file_buffer_->plooplen_ = plooplen;
  }
}
void GranularLooper::SetPinc(int pinc) {
  file_buffer_->pinc_ = pinc;
}

void GranularLooper::SetParam(std::string name, double val) {
  std::unique_ptr<FileBuffer> &buffer = file_buffer_;
  if (name == "active") {
    Start();
  } else if (name == "on") {
    eg_.StartEg();
  } else if (name == "off") {
    eg_.NoteOff();
  } else if (name == "speed")
    SetIncrSpeed(val);
  else if (name == "mode") {
    SetLoopMode(val);
  } else if (name == "idx") {
    if (val <= 100) {
      double pos = val / 100. * buffer->GetAudioBuffer()->size();
      buffer->SetAudioBufferReadIdx(pos);
    }
  } else if (name == "len")
    SetLoopLen(val);
  else if (name == "pidx")
    SetPidx(val);
  else if (name == "poffset")
    SetPOffset(val);
  else if (name == "plooplen")
    SetPlooplen(val);
  else if (name == "pinc")
    SetPinc(val);
  else if (name == "scramble")
    SetScramblePending();
  else if (name == "stutter")
    SetStutterPending();
  else if (name == "stop_in")
    SetStopPending(val);
  else if (name == "reverse")
    SetReversePending();
  else if (name == "speedulate")
    SetSpeedulatePending();
  else if (name == "gate")
    SetGatePending();
  else if (name == "slowdown")
    SetSlowdownPending();
  else if (name == "repeat")
    SetRepeatPending();
  else if (name == "strobe")
    SetStrobePending();
  else if (name == "pitch_ramp")
    SetPitchRampPending();
  else if (name == "octave_jump")
    SetOctaveJumpPending();
  else if (name == "pitch_staircase")
    SetPitchStaircasePending();
  else if (name == "grain_dur_ms")
    SetGrainDuration(val);
  else if (name == "grains_per_sec")
    SetGrainDensity(val);
  else if (name == "quasi_grain_fudge")
    SetQuasiGrainFudge(val);
  else if (name == "grain_spray_ms")
    SetGranularSpray(val);
  else if (name == "attack")
    eg_.SetAttackTimeMsec(val);
  else if (name == "decay")
    eg_.SetDecayTimeMsec(val);
  else if (name == "release")
    eg_.SetReleaseTimeMsec(val);
  else if (name == "pitch") {
    SetPitch(val);
  }
}

void GranularLooper::SetSubParam(int id, std::string name, double val) {
  file_buffer_->SetParam(name, val);
}

}  // namespace SBAudio
