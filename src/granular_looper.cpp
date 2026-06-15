#include "granular_looper.h"

#include <audioutils.h>
#include <libgen.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

#include "defjams.h"
#include "mixer.h"

namespace SBAudio {

namespace {

const std::array<std::string, 3> kLoopModeNames = {"LOOP", "STATIC", "SMUDGE"};

void ClearPattern(std::array<int, 16>& pattern) {
  for (int i = 0; i < 16; ++i) {
    pattern[i] = 0;
  }
}
void StutterPattern(std::array<int, 16>& pattern) {
  ClearPattern(pattern);

  int idx = 0;
  for (int i = 0; i < 16; ++i) {
    if (rand() % 100 > 70) idx++;
    pattern[i] = idx;
  }
}

void ScramblePattern(std::array<int, 16>& pattern) {
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

void SpeedulatePattern(std::array<int, 16>& pattern) {
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

void GatePattern(std::array<int, 16>& pattern) {
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

void SlowdownPattern(std::array<int, 16>& pattern) {
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

void RepeatPattern(std::array<int, 16>& pattern) {
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

void StrobePattern(std::array<int, 16>& pattern) {
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

void ClearPitchPattern(std::array<double, 16>& pattern) {
  for (int i = 0; i < 16; ++i) {
    pattern[i] = 1.0;
  }
}

void PitchRampPattern(std::array<double, 16>& pattern) {
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

void OctaveJumpPattern(std::array<double, 16>& pattern) {
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

void PitchStaircasePattern(std::array<double, 16>& pattern) {
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

// ─────────────────────────────────────────────────────────────────────────────
// shhh helpers

double GranularLooper::ComputeBufferRMS(const std::vector<double>* buf,
                                        int num_ch, int start_idx,
                                        int window_frames) {
  int total = (int)buf->size();
  if (total == 0 || window_frames == 0) return 0.0;
  double sum = 0.0;
  int n = window_frames * num_ch;
  for (int i = 0; i < n; i++) {
    double s = (*buf)[(start_idx + i) % total];
    sum += s * s;
  }
  return std::sqrt(sum / n);
}

FileBuffer* GranularLooper::SelectShhhBuffer(bool want_loudest) {
  FileBuffer* best = file_buffers_[primary_buf_idx_].get();
  double best_rms = want_loudest ? -1.0 : std::numeric_limits<double>::max();
  int read_idx = file_buffers_[primary_buf_idx_]->audio_buffer_read_idx_.load();
  int window = shhh_window_frames_ > 0 ? shhh_window_frames_
                                       : engine_.grain_duration_frames();
  for (auto& fb : file_buffers_) {
    const std::vector<double>* buf = fb->GetAudioBuffer();
    if (!buf || buf->empty()) continue;
    double rms = ComputeBufferRMS(buf, fb->num_channels_, read_idx, window);
    if ((want_loudest && rms > best_rms) || (!want_loudest && rms < best_rms)) {
      best_rms = rms;
      best = fb.get();
    }
  }
  return best;
}

// ─────────────────────────────────────────────────────────────────────────────

void GranularLooper::Reset() {
  if (file_buffers_.empty()) return;
  file_buffers_[primary_buf_idx_]->audio_buffer_read_idx_ = 0;
  engine_.Reset();
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
}

GranularLooper::GranularLooper(std::string filename, unsigned int loop_mode)
    : grain_type_{SoundGrainType::Sample} {
  file_buffers_.reserve(
      8);  // prevent reallocation on audio thread during add_buf
  file_buffers_.push_back(std::make_unique<FileBuffer>(filename));
  type = LOOPER_TYPE;
  SetLoopMode(loop_mode);
  Reset();
}

GranularLooper::~GranularLooper() {
  // TODO delete file
}

void GranularLooper::AddBuffer(std::unique_ptr<FileBuffer> fb) {
  file_buffers_.push_back(std::move(fb));
}

void GranularLooper::EventNotify(broadcast_event event,
                                 mixer_timing_info tinfo) {
  (void)event;
  if (file_buffers_.empty()) return;
  auto& primary = file_buffers_[primary_buf_idx_];

  if (!started_ && tinfo.is_start_of_loop) {
    LaunchGrain(tinfo);
    eg_.StartEg();
    started_ = true;
  }
  if (!started_) return;

  const std::vector<double>* audio_buffer = primary->GetAudioBuffer();

  if (tinfo.is_midi_tick) {
    primary->cur_midi_idx_ =
        fmodf(primary->cur_midi_idx_ + primary->incr_speed_,
              PPBAR * primary->loop_len_);
    if (primary->loop_mode_ == LoopMode::loop_mode) {
      double decimal_percent_of_loop =
          primary->cur_midi_idx_ / (PPBAR * primary->loop_len_);
      double new_read_idx = decimal_percent_of_loop * audio_buffer->size();
      if (reverse_mode_)
        new_read_idx = (audio_buffer->size() - 1) - new_read_idx;

      // size_of_sixteenth_ is scaled by loop_len_, so recover the 1-bar
      // sixteenth size for ploop region bounds — otherwise a 2-bar loop wraps
      // at half the buffer instead of the full buffer.
      double one_bar_sixteenth =
          primary->size_of_sixteenth_ * primary->loop_len_;
      new_read_idx =
          fmodf((fmodf(new_read_idx, one_bar_sixteenth * primary->plooplen_) +
                 primary->poffset_ * one_bar_sixteenth),
                audio_buffer->size());

      // this ensures new_read_idx is even
      if (primary->num_channels_ == 2) new_read_idx -= ((int)new_read_idx & 1);

      if (new_read_idx < 0 || new_read_idx > (int)audio_buffer->size() - 1) {
        new_read_idx = 0;
        std::cout << "OH YA:" << new_read_idx
                  << " bufflen:" << (audio_buffer->size() - 1) << std::endl;
      }

      primary->audio_buffer_read_idx_ = new_read_idx;

      int read_idx = primary->audio_buffer_read_idx_.load();
      int rel_pos_within_a_sixteenth =
          fmod(static_cast<double>(read_idx), primary->size_of_sixteenth_);

      if (primary->stutter_mode_ || primary->scramble_mode_ ||
          primary->speedulate_mode_ || primary->slowdown_mode_ ||
          primary->repeat_mode_ || primary->strobe_mode_) {
        int target_slice = primary->scrambled_pattern_[primary->cur_sixteenth_];
        int normal_slice = primary->cur_sixteenth_;

        // If FX remapped to a different slice, snap to its start
        // for a punchier transient. Otherwise keep the sub-position.
        int offset =
            (target_slice != normal_slice) ? 0 : rel_pos_within_a_sixteenth;

        primary->audio_buffer_read_idx_ = static_cast<int>(
            fmodf((target_slice * primary->size_of_sixteenth_) + offset,
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
    if (primary->scramble_pending_) {
      primary->scramble_mode_ = true;
      primary->scramble_pending_ = false;
    } else
      primary->scramble_mode_ = false;

    if (primary->stutter_pending_) {
      primary->stutter_mode_ = true;
      primary->stutter_pending_ = false;
    } else
      primary->stutter_mode_ = false;

    if (primary->speedulate_pending_) {
      primary->speedulate_mode_ = true;
      primary->speedulate_pending_ = false;
    } else
      primary->speedulate_mode_ = false;

    if (primary->gate_pending_) {
      primary->gate_mode_ = true;
      primary->gate_pending_ = false;
    } else
      primary->gate_mode_ = false;

    if (primary->slowdown_pending_) {
      primary->slowdown_mode_ = true;
      primary->slowdown_pending_ = false;
    } else
      primary->slowdown_mode_ = false;

    if (primary->repeat_pending_) {
      primary->repeat_mode_ = true;
      primary->repeat_pending_ = false;
    } else
      primary->repeat_mode_ = false;

    if (primary->strobe_pending_) {
      primary->strobe_mode_ = true;
      primary->strobe_pending_ = false;
    } else
      primary->strobe_mode_ = false;

    if (primary->pitch_ramp_pending_) {
      primary->pitch_ramp_mode_ = true;
      primary->pitch_ramp_pending_ = false;
    } else
      primary->pitch_ramp_mode_ = false;

    if (primary->octave_jump_pending_) {
      primary->octave_jump_mode_ = true;
      primary->octave_jump_pending_ = false;
    } else
      primary->octave_jump_mode_ = false;

    if (primary->pitch_staircase_pending_) {
      primary->pitch_staircase_mode_ = true;
      primary->pitch_staircase_pending_ = false;
    } else
      primary->pitch_staircase_mode_ = false;

    if (!primary->pitch_ramp_mode_ && !primary->octave_jump_mode_ &&
        !primary->pitch_staircase_mode_) {
      ClearPitchPattern(primary->pitch_pattern_);
    }

    if (reverse_pending_) {
      reverse_mode_ = true;
      reverse_pending_ = false;
    } else
      reverse_mode_ = false;
  }

  if (tinfo.is_sixteenth) {
    primary->cur_sixteenth_ = tinfo.sixteenth_note_tick % 16;
  }
}

// for debugging only
int launch_count = 0;
int samp_diff = 0;
int midi_diff = 0;

void GranularLooper::LaunchGrain(mixer_timing_info tinfo) {
  if (file_buffers_.empty()) return;
  auto& primary = file_buffers_[primary_buf_idx_];
  int read_idx = primary->audio_buffer_read_idx_;
  double pitch = primary->pitch_ratio_.load() *
                 primary->pitch_pattern_[primary->cur_sixteenth_];

  if (shhh_mode_ > 0 && file_buffers_.size() > 1) {
    // shhh: pick one buffer by RMS loudness, use its own pitch and gain
    bool want_loudest = (shhh_mode_ == 2);
    FileBuffer* buf = SelectShhhBuffer(want_loudest);
    double buf_pitch = buf->pitch_ratio_.load() *
                       primary->pitch_pattern_[primary->cur_sixteenth_];
    engine_.LaunchGrain(read_idx, buf->GetAudioBuffer(), buf->num_channels_,
                        buf_pitch, tinfo.cur_sample, reverse_mode_, degrade_by_,
                        buf->gain_, buf->distortion_);
  } else if (!xf_left_.empty() || !xf_right_.empty()) {
    // xfader: launch from every assigned buffer with its xfader gain
    double t = (xfader_pos_ + 1.0) * 0.5;  // 0=full left, 1=full right
    auto launch_group = [&](const std::vector<int>& group, double xf_gain) {
      for (int idx : group) {
        if (idx < 0 || idx >= (int)file_buffers_.size()) continue;
        auto& fb = file_buffers_[idx];
        double buf_pitch = fb->pitch_ratio_.load() *
                           primary->pitch_pattern_[primary->cur_sixteenth_];
        engine_.LaunchGrain(read_idx, fb->GetAudioBuffer(), fb->num_channels_,
                            buf_pitch, tinfo.cur_sample, reverse_mode_,
                            degrade_by_, fb->gain_ * xf_gain, fb->distortion_);
      }
    };
    launch_group(xf_left_, std::cos(t * M_PI * 0.5));
    launch_group(xf_right_, std::sin(t * M_PI * 0.5));
  } else {
    engine_.LaunchGrain(read_idx, primary->GetAudioBuffer(),
                        primary->num_channels_, pitch, tinfo.cur_sample,
                        reverse_mode_, degrade_by_, primary->gain_,
                        primary->distortion_);
  }
}

StereoVal GranularLooper::GenNext(mixer_timing_info tinfo) {
  if (!started_ || !active) return {0., 0.};
  if (file_buffers_.empty()) return {0., 0.};
  auto& primary = file_buffers_[primary_buf_idx_];

  if (stop_pending_ && eg_.m_state == OFFF) active = false;

  // Smooth xfader position ramp
  if (xfader_pos_ != xfader_target_) {
    double diff = xfader_target_ - xfader_pos_;
    double step = std::min(std::abs(diff), xfader_ramp_rate_);
    xfader_pos_ += (diff > 0.0 ? step : -step);
  }

  if (engine_.ShouldLaunch(tinfo.cur_sample)) {
    LaunchGrain(tinfo);
  }

  StereoVal val = engine_.SumGrains();

  eg_.Update();
  double eg_amp = eg_.DoEnvelope(NULL);

  pan = fmin(pan, 1.0);
  pan = fmax(pan, -1.0);
  double pan_left = 0.707;
  double pan_right = 0.707;
  calculate_pan_values(pan, &pan_left, &pan_right);

  val.left = val.left * volume * eg_amp * pan_left;
  val.right = val.right * volume * eg_amp * pan_right;

  if (primary->gate_mode_) {
    int cur_step = primary->cur_sixteenth_;
    int gate_open = primary->gate_pattern_[cur_step];
    int sixteenth_size = primary->size_of_sixteenth_;
    int read_idx = primary->audio_buffer_read_idx_.load();
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
  if (file_buffers_.empty()) return "GranularLooper (no buffer)";
  auto& primary = file_buffers_[primary_buf_idx_];
  std::stringstream ss;
  if (!active || volume == 0)
    ss << ANSI_COLOR_RESET;
  else
    ss << ANSI_COLOR_RED;
  ss << "SBPlayer // " << primary->filename_ << "\n";
  ss << "vol:" << volume << " pan:" << pan << " pitch:" << primary->pitch_ratio_
     << " idx:"
     << (int)(100. / primary->GetAudioBuffer()->size() *
              primary->audio_buffer_read_idx_)
     << " mode:" << kLoopModeNames[primary->loop_mode_] << "("
     << primary->loop_mode_ << ")"
     << " len:" << primary->loop_len_;
  if (file_buffers_.size() > 1) ss << " bufs:" << file_buffers_.size();
  if (shhh_mode_ > 0) {
    const char* modes[] = {"off", "quietest", "loudest"};
    ss << " shhh:" << modes[shhh_mode_];
  }
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

std::string GranularLooper::Info() {
  const char* INSTRUMENT_COLOR = active ? ANSI_COLOR_RED : ANSI_COLOR_RESET;

  std::stringstream ss;

  if (file_buffers_.empty()) {
    ss << INSTRUMENT_COLOR << "\nLooper (no buffer loaded)\n"
       << ANSI_COLOR_RESET;
    return ss.str();
  }

  // ── line 1: type  mixer  mode  shhh ─────────────────────────────────────
  const char* shhh_labels[] = {"off", "quietest", "loudest"};
  auto& primary = file_buffers_[primary_buf_idx_];
  ss << "\n"
     << INSTRUMENT_COLOR << "Looper" << ANSI_COLOR_RESET << "  vol:" << volume
     << " pan:" << pan << "  mode:" << kLoopModeNames[primary->loop_mode_]
     << "  shhh:" << shhh_labels[shhh_mode_];
  if (shhh_window_frames_ > 0)
    ss << " shhh_win:" << std::fixed << std::setprecision(1)
       << (shhh_window_frames_ / 44.1) << "ms" << std::defaultfloat;
  else
    ss << " shhh_win:grain_dur";
  ss << "\n";

  // ── granular engine ──────────────────────────────────────────────────────
  ss << "grain_dur_ms:" << std::fixed << std::setprecision(1)
     << (engine_.grain_duration_frames() / 44.1) << std::defaultfloat
     << "  grains_per_sec:" << engine_.grains_per_sec()
     << "  grain_overlap:" << engine_.grain_overlap()
     << "  grain_env:" << engine_.envelope_shape() << "\n"
     << "quasi_grain_fudge:" << engine_.quasi_grain_fudge()
     << "  grain_spray_ms:" << std::fixed << std::setprecision(1)
     << (engine_.granular_spray_frames() / 44.1) << std::defaultfloat << "\n";

  // ── envelope ─────────────────────────────────────────────────────────────
  ss << "attack:" << (int)eg_.m_attack_time_msec
     << "  decay:" << (int)eg_.m_decay_time_msec
     << "  release:" << (int)eg_.m_release_time_msec << "\n";

  // ── buffers ───────────────────────────────────────────────────────────────
  bool multi = file_buffers_.size() > 1;
  for (int i = 0; i < (int)file_buffers_.size(); i++) {
    auto& fb = file_buffers_[i];
    std::string fname = fb->filename_;
    size_t sep = fname.rfind('/');
    if (sep != std::string::npos) fname = fname.substr(sep + 1);

    ss << ANSI_COLOR_WHITE << "buf[" << i << "]  " << fname;
    if (multi && i == primary_buf_idx_) ss << "  [primary]";
    // show which xfader side this buffer is on
    bool in_left =
        std::find(xf_left_.begin(), xf_left_.end(), i) != xf_left_.end();
    bool in_right =
        std::find(xf_right_.begin(), xf_right_.end(), i) != xf_right_.end();
    if (in_left) ss << "  [xf:L]";
    if (in_right) ss << "  [xf:R]";
    ss << "\n"
       << "  pitch:" << fb->pitch_ratio_.load() << " speed:" << fb->incr_speed_
       << " gain:" << fb->gain_ << " distort:" << fb->distortion_
       << " len:" << fb->loop_len_ << " poffset:" << fb->poffset_
       << " plooplen:" << (int)fb->plooplen_ << " pinc:" << fb->pinc_ << "\n";
  }

  // xfader block — only shown when active
  if (!xf_left_.empty() || !xf_right_.empty()) {
    constexpr int kBarWidth = 15;
    int pos = (int)((xfader_pos_ + 1.0) * 0.5 * kBarWidth);
    pos = std::max(0, std::min(kBarWidth, pos));
    ss << COOL_COLOR_PINK2 << "xfader: [L";
    for (int idx : xf_left_) ss << idx;
    ss << "] <";
    for (int k = 0; k < pos; k++) ss << "-";
    ss << "I";
    for (int k = pos; k < kBarWidth; k++) ss << "-";
    ss << "> [R";
    for (int idx : xf_right_) ss << idx;
    ss << "]  xfpos:" << xfader_pos_ << "  xfspeed:" << xfader_ramp_rate_
       << "\n";
  }
  ss << ANSI_COLOR_RESET;

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
  if (dur > 0) engine_.SetDensity(1000. / dur);
}

void GranularLooper::SetGrainDensity(int gps) {
  engine_.SetDensity(gps);
}

void GranularLooper::SetGrainOverlap(double overlap) {
  engine_.SetOverlap(overlap);
}

void GranularLooper::SetGrainEnvShape(int shape) {
  engine_.SetEnvShape(shape);
}

void GranularLooper::SetGranularSpray(int spray_ms) {
  engine_.SetSpray(spray_ms / 1000.0 * SAMPLE_RATE);
}

void GranularLooper::SetQuasiGrainFudge(int fudgefactor) {
  engine_.SetFudge(fudgefactor);
}

void GranularLooper::SetPitch(double pitch_ratio) {
  if (!file_buffers_.empty())
    file_buffers_[primary_buf_idx_]->SetPitch(pitch_ratio);
}

void GranularLooper::SetIncrSpeed(double speed) {
  if (!file_buffers_.empty())
    file_buffers_[primary_buf_idx_]->incr_speed_ = speed;
}

void GranularLooper::SetReverseMode(bool b) {
  reverse_mode_ = b;
}

void GranularLooper::SetLoopMode(unsigned int m) {
  if (file_buffers_.empty()) return;
  auto& buffer = file_buffers_[primary_buf_idx_];
  switch (m) {
    case (0):
      buffer->loop_mode_ = LoopMode::loop_mode;
      engine_.SetFudge(0);
      engine_.SetSpray(0);
      break;
    case (1):
      buffer->loop_mode_ = LoopMode::static_mode;
      engine_.SetFudge(0);
      engine_.SetSpray(0);
      engine_.SetOverlap(0.5);
      engine_.SetEnvShape(1);
      break;
    case (2):
    default:
      buffer->loop_mode_ = LoopMode::smudge_mode;
      engine_.SetFudge(220);
      engine_.SetSpray(441);  // 10ms * (44100/1000)
      engine_.SetEnvShape(1);
      engine_.SetOverlap(0.8);
  }
}

void GranularLooper::SetScramblePending() {
  if (file_buffers_.empty()) return;
  auto& buffer = file_buffers_[primary_buf_idx_];
  buffer->scramble_pending_ = true;
  ScramblePattern(buffer->scrambled_pattern_);
}

void GranularLooper::SetStopPending(int loops) {
  stop_count_pending_ = true;
  stop_len_ = loops;
  stop_countr_ = 0;
}

void GranularLooper::SetStutterPending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->stutter_pending_ = true;
    StutterPattern(file_buffers_[primary_buf_idx_]->scrambled_pattern_);
  }
}

void GranularLooper::SetReversePending() {
  reverse_pending_ = true;
}

void GranularLooper::SetSpeedulatePending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->speedulate_pending_ = true;
    SpeedulatePattern(file_buffers_[primary_buf_idx_]->scrambled_pattern_);
  }
}

void GranularLooper::SetGatePending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->gate_pending_ = true;
    GatePattern(file_buffers_[primary_buf_idx_]->gate_pattern_);
  }
}

void GranularLooper::SetSlowdownPending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->slowdown_pending_ = true;
    SlowdownPattern(file_buffers_[primary_buf_idx_]->scrambled_pattern_);
  }
}

void GranularLooper::SetRepeatPending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->repeat_pending_ = true;
    RepeatPattern(file_buffers_[primary_buf_idx_]->scrambled_pattern_);
  }
}

void GranularLooper::SetStrobePending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->strobe_pending_ = true;
    StrobePattern(file_buffers_[primary_buf_idx_]->scrambled_pattern_);
  }
}

void GranularLooper::SetPitchRampPending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->pitch_ramp_pending_ = true;
    PitchRampPattern(file_buffers_[primary_buf_idx_]->pitch_pattern_);
  }
}

void GranularLooper::SetOctaveJumpPending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->octave_jump_pending_ = true;
    OctaveJumpPattern(file_buffers_[primary_buf_idx_]->pitch_pattern_);
  }
}

void GranularLooper::SetPitchStaircasePending() {
  if (!file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->pitch_staircase_pending_ = true;
    PitchStaircasePattern(file_buffers_[primary_buf_idx_]->pitch_pattern_);
  }
}

void GranularLooper::SetLoopLen(double bars) {
  if (bars != 0 && !file_buffers_.empty()) {
    file_buffers_[primary_buf_idx_]->SetLoopLen(bars);
  }
}

void GranularLooper::SetDegradeBy(int degradation) {
  if (degradation >= 0 && degradation <= 100) degrade_by_ = degradation;
}

void GranularLooper::NoteOn(midi_event ev) {
  if (file_buffers_.empty()) return;
  auto& primary = file_buffers_[primary_buf_idx_];
  started_ = true;
  int sixteenth_pos = ev.data1 % 16;
  primary->cur_midi_idx_ = sixteenth_pos * PPSIXTEENTH;

  // Snap to the exact start of the sixteenth slice
  double new_read_idx = sixteenth_pos * primary->size_of_sixteenth_;

  // Ensure stereo alignment
  if (primary->num_channels_ == 2) new_read_idx -= ((int)new_read_idx & 1);

  primary->audio_buffer_read_idx_ = static_cast<int>(new_read_idx);
  engine_.SetReadIdx(new_read_idx);
  engine_.Reset();  // forces immediate grain launch on next sample
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
  if (!file_buffers_.empty())
    file_buffers_[primary_buf_idx_]->poffset_ =
        abs(val - file_buffers_[primary_buf_idx_]->cur_sixteenth_) % 16;
}

void GranularLooper::SetPOffset(int p) {
  if (!file_buffers_.empty() && p >= 0 && p <= 15) {
    file_buffers_[primary_buf_idx_]->poffset_ = p;
  }
}

void GranularLooper::SetPlooplen(int plooplen) {
  if (!file_buffers_.empty() && plooplen > 0 && plooplen <= 16) {
    file_buffers_[primary_buf_idx_]->plooplen_ = plooplen;
  }
}

void GranularLooper::SetPinc(int pinc) {
  if (!file_buffers_.empty()) file_buffers_[primary_buf_idx_]->pinc_ = pinc;
}

void GranularLooper::SetParam(std::string name, double val) {
  if (name == "active") {
    Start();
  } else if (name == "on") {
    eg_.StartEg();
  } else if (name == "off") {
    eg_.NoteOff();
  } else if (name == "speed") {
    SetIncrSpeed(val);
  } else if (name == "mode") {
    SetLoopMode(val);
  } else if (name == "idx") {
    if (!file_buffers_.empty() && val <= 100) {
      double pos = val / 100. *
                   file_buffers_[primary_buf_idx_]->GetAudioBuffer()->size();
      file_buffers_[primary_buf_idx_]->SetAudioBufferReadIdx(pos);
    }
  } else if (name == "len") {
    SetLoopLen(val);
  } else if (name == "pidx") {
    SetPidx(val);
  } else if (name == "poffset") {
    SetPOffset(val);
  } else if (name == "plooplen") {
    SetPlooplen(val);
  } else if (name == "pinc") {
    SetPinc(val);
  } else if (name == "scramble") {
    SetScramblePending();
  } else if (name == "stutter") {
    SetStutterPending();
  } else if (name == "stop_in") {
    SetStopPending(val);
  } else if (name == "reverse") {
    SetReversePending();
  } else if (name == "speedulate") {
    SetSpeedulatePending();
  } else if (name == "gate") {
    SetGatePending();
  } else if (name == "slowdown") {
    SetSlowdownPending();
  } else if (name == "repeat") {
    SetRepeatPending();
  } else if (name == "strobe") {
    SetStrobePending();
  } else if (name == "pitch_ramp") {
    SetPitchRampPending();
  } else if (name == "octave_jump") {
    SetOctaveJumpPending();
  } else if (name == "pitch_staircase") {
    SetPitchStaircasePending();
  } else if (name == "grain_dur_ms") {
    SetGrainDuration(val);
  } else if (name == "grains_per_sec") {
    SetGrainDensity(val);
  } else if (name == "quasi_grain_fudge") {
    SetQuasiGrainFudge(val);
  } else if (name == "grain_spray_ms") {
    SetGranularSpray(val);
  } else if (name == "attack") {
    eg_.SetAttackTimeMsec(val);
  } else if (name == "decay") {
    eg_.SetDecayTimeMsec(val);
  } else if (name == "release") {
    eg_.SetReleaseTimeMsec(val);
  } else if (name == "pitch") {
    SetPitch(val);
  } else if (name == "grain_overlap") {
    SetGrainOverlap(val);
  } else if (name == "grain_env") {
    SetGrainEnvShape(val);
  } else if (name == "shhh") {
    shhh_mode_ = std::max(0, std::min(2, (int)val));
  } else if (name == "shhh_window_ms" || name == "window" ||
             name == "shhh_win") {
    shhh_window_frames_ = (int)(val * SAMPLE_RATE / 1000.0);
  } else if (name == "primary") {
    int idx = (int)val;
    if (idx >= 0 && idx < (int)file_buffers_.size()) primary_buf_idx_ = idx;
  } else if (name == "xfl") {
    int idx = (int)val;
    if (idx >= 0 && idx < (int)file_buffers_.size()) xf_left_.push_back(idx);
  } else if (name == "xfr") {
    int idx = (int)val;
    if (idx >= 0 && idx < (int)file_buffers_.size()) xf_right_.push_back(idx);
  } else if (name == "xfpos") {
    xfader_target_ = std::max(-1.0, std::min(1.0, val));
  } else if (name == "xfspeed") {
    xfader_ramp_rate_ = std::max(0.0001, val);
  } else if (name == "xfclear") {
    xf_left_.clear();
    xf_right_.clear();
    xfader_pos_ = 0.0;
    xfader_target_ = 0.0;
  }
}

void GranularLooper::SetSubParam(int id, std::string name, double val) {
  // id selects which buffer to configure (0 = primary)
  if (id >= 0 && id < (int)file_buffers_.size())
    file_buffers_[id]->SetParam(name, val);
}

}  // namespace SBAudio
