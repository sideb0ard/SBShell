#pragma once

#include <string>
#include <vector>

#include "soundgenerator.h"

namespace SBAudio {

class ImageSounder : public SoundGenerator {
 public:
  ImageSounder(std::string filepath);
  ~ImageSounder() = default;

  StereoVal GenNext(mixer_timing_info tinfo) override;
  std::string Info() override;
  std::string Status() override;
  void SetParam(std::string name, double val) override;

 private:
  std::string filepath_;
  int width_{0};
  int height_{0};
  std::vector<std::vector<float>> pixels_;  // [col][row], brightness 0..1

  int col_idx_{0};
  int col_sample_{0};
  int samples_per_col_{512};

  double lo_freq_{80};
  double hi_freq_{8000.0};
  std::vector<double> phases_;    // one phase accum per image row
  std::vector<float> col_norms_;  // per-column RMS energy for normalization
  double gain_{0.3};              // output scale — boost/cut beyond 0-1 vol

  int fade_samples_{64};  // crossfade length on column transition
  int fade_idx_{-1};      // -1 = not fading; 0..fade_samples_-1 = fading
  int prev_col_idx_{0};   // column we're fading away from
};

}  // namespace SBAudio
