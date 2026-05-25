#pragma once

#include <array>
#include <vector>

#include "grains.h"

namespace SBAudio {

// GranularEngine — shared grain pool logic used by both GranularLooper
// (file-based) and Granulate FX (live capture). Callers provide the audio
// source buffer and read position; the engine manages the pool, timing,
// envelopes, and mixing.
class GranularEngine {
 public:
  GranularEngine();

  // Reset pool and timing — call on start or mode change
  void Reset();

  // Returns true when it's time to launch the next grain
  bool ShouldLaunch(int cur_sample) const;

  // Launch a grain from the given buffer at read_idx.
  // Updates next_launch_sample_ and grain_spacing_frames_ internally.
  void LaunchGrain(int read_idx, std::vector<double>* audio_buffer,
                   int num_channels, double pitch_ratio, int cur_sample,
                   bool reverse_mode = false, int degrade_by = 0);

  // Iterate all active grains, apply envelopes, sum and return mixed output
  StereoVal SumGrains();

  // Snap all grain read positions (e.g. on NoteOn jump)
  void SetReadIdx(int idx);

  // Parameter setters — all recompute spacing as needed
  void SetDensity(int grains_per_sec);
  void SetOverlap(double overlap);  // 0.0–0.9
  void SetEnvShape(int shape);      // 0=Tukey, 1=Hann
  void SetSpray(int spray_frames);
  void SetFudge(int fudge);

  // Read-only state for Info()/Status()
  int grains_per_sec() const {
    return grains_per_sec_;
  }
  int grain_duration_frames() const {
    return grain_duration_frames_;
  }
  double grain_overlap() const {
    return grain_overlap_;
  }
  int envelope_shape() const {
    return envelope_shape_;
  }
  int granular_spray_frames() const {
    return granular_spray_frames_;
  }
  int quasi_grain_fudge() const {
    return quasi_grain_fudge_;
  }

 private:
  static constexpr int kMaxGrains = 16;
  std::array<SoundGrainSample, kMaxGrains> grain_pool_;

  int grains_per_sec_{0};
  int grain_duration_frames_{0};
  int grain_spacing_frames_{0};
  double grain_overlap_{0.2};
  int envelope_shape_{0};
  int granular_spray_frames_{0};
  int quasi_grain_fudge_{0};

  int next_launch_sample_{1};

  void RecomputeSpacing();
};

}  // namespace SBAudio
