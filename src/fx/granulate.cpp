#include "granulate.h"

#include <algorithm>
#include <sstream>

#include "defjams.h"

Granulate::Granulate()
    : capture_buf_(kCaptureSecs * SAMPLE_RATE * kCaptureChannels, 0.0) {
  type_ = fx_type::GRANULATE;
  enabled_ = true;
  engine_.SetDensity(15);
  engine_.SetOverlap(0.2);
}

StereoVal Granulate::Process(StereoVal input) {
  // Write incoming stereo sample into the ring capture buffer
  capture_buf_[write_idx_] = input.left;
  capture_buf_[write_idx_ + 1] = input.right;
  write_idx_ = (write_idx_ + 2) % capture_buf_.size();
  cur_sample_++;

  // Launch grains reading slightly behind the write head so they always
  // have captured audio to read. Offset = one grain duration worth of frames.
  if (engine_.ShouldLaunch(cur_sample_)) {
    int lookahead = engine_.grain_duration_frames() * kCaptureChannels;
    int read_idx = ((int)capture_buf_.size() + write_idx_ - lookahead) %
                   (int)capture_buf_.size();
    // Align to stereo pair
    if (read_idx & 1) read_idx--;

    engine_.LaunchGrain(read_idx, &capture_buf_, kCaptureChannels,
                        /*pitch_ratio=*/1.0, cur_sample_);
  }

  StereoVal wet = engine_.SumGrains();
  return {input.left * (1.0 - wet_) + wet.left * wet_,
          input.right * (1.0 - wet_) + wet.right * wet_};
}

void Granulate::EventNotify(broadcast_event event, mixer_timing_info tinfo) {
  (void)event;
  (void)tinfo;
}

std::string Granulate::Status() {
  std::stringstream ss;
  ss << "granulate wet:" << wet_ << " overlap:" << engine_.grain_overlap()
     << " env:" << engine_.envelope_shape()
     << " density:" << engine_.grains_per_sec()
     << " spray:" << engine_.granular_spray_frames() / 44.1;
  return ss.str();
}

void Granulate::SetParam(std::string name, double val) {
  if (name == "wet") {
    wet_ = std::max(0.0, std::min(1.0, val));
  } else if (name == "grain_overlap" || name == "overlap") {
    engine_.SetOverlap(val);
  } else if (name == "grain_env" || name == "env") {
    engine_.SetEnvShape((int)val);
  } else if (name == "grains_per_sec" || name == "density") {
    engine_.SetDensity((int)val);
  } else if (name == "grain_dur_ms" || name == "dur_ms") {
    if (val > 0) engine_.SetDensity(1000. / val);
  } else if (name == "grain_spray_ms" || name == "spray") {
    engine_.SetSpray(val / 1000.0 * SAMPLE_RATE);
  } else if (name == "quasi_grain_fudge" || name == "fudge") {
    engine_.SetFudge((int)val);
  }
}
