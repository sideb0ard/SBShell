#include "granular_engine.h"

#include <algorithm>
#include <cmath>

#include "defjams.h"

namespace SBAudio {

GranularEngine::GranularEngine() {
  for (auto& g : grain_pool_) {
    g.active = false;
    g.type = SoundGrainType::Sample;
  }
}

void GranularEngine::Reset() {
  for (auto& g : grain_pool_) g.active = false;
  next_launch_sample_ = 1;
}

bool GranularEngine::ShouldLaunch(int cur_sample) const {
  return cur_sample >= next_launch_sample_;
}

void GranularEngine::LaunchGrain(int read_idx,
                                 std::vector<double>* audio_buffer,
                                 int num_channels, double pitch_ratio,
                                 int cur_sample, bool reverse_mode,
                                 int degrade_by, int channel_mask) {
  // Find a free slot
  SoundGrainSample* slot = nullptr;
  for (auto& g : grain_pool_) {
    if (!g.active) {
      slot = &g;
      break;
    }
  }
  if (!slot) return;  // all slots busy — skip this launch
  if (!audio_buffer || audio_buffer->empty()) return;

  int duration_frames = grain_duration_frames_;
  if (quasi_grain_fudge_ > 0)
    duration_frames += rand() % (int)(quasi_grain_fudge_ * 44.1);

  int spray_idx = read_idx;
  if (granular_spray_frames_ > 0 && !audio_buffer->empty())
    spray_idx += (rand() % granular_spray_frames_) % audio_buffer->size();

  SoundGrainParams params = {
      .grain_type = SoundGrainType::Sample,
      .dur_frames = duration_frames,
      .starting_idx = spray_idx,
      .reverse_mode = reverse_mode,
      .num_channels = num_channels,
      .degrade_by = degrade_by,
      .pitch_ratio = pitch_ratio,
      .audio_buffer = audio_buffer,
      .envelope_shape = envelope_shape_,
      .overlap_fraction = grain_overlap_,
      .channel_mask = channel_mask,
  };
  slot->Initialize(params);

  grain_spacing_frames_ = duration_frames * (1.0 - grain_overlap_);
  next_launch_sample_ = cur_sample + grain_spacing_frames_;
}

StereoVal GranularEngine::SumGrains() {
  StereoVal val = {0.0, 0.0};
  int left_count = 0, right_count = 0;
  for (auto& g : grain_pool_) {
    if (g.active) {
      StereoVal gv = g.Generate();
      val.left += gv.left;
      val.right += gv.right;
      if (g.channel_mask != 2) left_count++;   // stereo or left-only
      if (g.channel_mask != 1) right_count++;  // stereo or right-only
    }
  }
  if (left_count > 1) val.left /= std::sqrt(static_cast<double>(left_count));
  if (right_count > 1) val.right /= std::sqrt(static_cast<double>(right_count));
  return val;
}

void GranularEngine::SetReadIdx(int idx) {
  for (auto& g : grain_pool_) g.SetReadIdx(idx);
}

void GranularEngine::SetDensity(int grains_per_sec) {
  grains_per_sec_ = std::max(1, grains_per_sec);
  grain_duration_frames_ = (double)SAMPLE_RATE / grains_per_sec_;
  RecomputeSpacing();
  next_launch_sample_ = 0;  // trigger immediately on next sample
}

void GranularEngine::SetOverlap(double overlap) {
  grain_overlap_ = std::max(0.0, std::min(0.9, overlap));
  RecomputeSpacing();
}

void GranularEngine::SetEnvShape(int shape) {
  envelope_shape_ = (shape == 1) ? 1 : 0;
}

void GranularEngine::SetSpray(int spray_frames) {
  granular_spray_frames_ = spray_frames;
}

void GranularEngine::SetFudge(int fudge) {
  quasi_grain_fudge_ = fudge;
}

void GranularEngine::RecomputeSpacing() {
  if (grain_duration_frames_ > 0)
    grain_spacing_frames_ = grain_duration_frames_ * (1.0 - grain_overlap_);
}

}  // namespace SBAudio
