#pragma once

#include <vector>

#include "fx/fx.h"
#include "granular_engine.h"

class Granulate : public Fx {
 public:
  Granulate();
  ~Granulate() = default;
  std::string Status() override;
  StereoVal Process(StereoVal input) override;
  void SetParam(std::string name, double val) override;
  void EventNotify(broadcast_event event, mixer_timing_info tinfo) override;

 private:
  // Capture buffer: live audio written as a ring buffer; grains read from it
  // exactly like GranularLooper reads from a file buffer.
  static constexpr int kCaptureSecs = 10;
  static constexpr int kCaptureChannels = 2;
  std::vector<double> capture_buf_;
  int write_idx_{0};
  int cur_sample_{0};

  double wet_{0.5};

  SBAudio::GranularEngine engine_;
};
