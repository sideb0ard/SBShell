#include "filter_sem.h"

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "defjams.h"

FilterSem::FilterSem() {
  // aux_control_ = 0.5;
  aux_control_ = 1;
  filter_type_ = LPF2;

  alpha_ = 1.0;
  alpha0_ = 1.0;
  rho_ = 1.0;

  Reset();
}

void FilterSem::Reset() {
  z11_ = 0;
  z12_ = 0;
}

void FilterSem::SetQControl(double qcontrol) {
  q_ = (25.0 - 0.5) * (qcontrol - 1.0) / (10.0 - 1.0) + 0.5;
}

void FilterSem::Update() {
  Filter::Update();  // update base class

  double wd = 2 * M_PI * fc_;
  double T = 1.0 / SAMPLE_RATE;
  double wa = (2 / T) * tan(wd * T / 2);
  double g = wa * T / 2;

  double R = 1.0 / (2.0 * q_);
  alpha0_ = 1.0 / (1.0 + 2.0 * R * g + g * g);
  alpha_ = g;
  rho_ = 2.0 * R + g;
}

double FilterSem::DoFilter(double xn) {
  if (filter_type_ != LPF2 && filter_type_ != HPF2 && filter_type_ != BPF2 &&
      filter_type_ != BSF2) {
    std::cerr << "SORRY BUD, NOT SUPPORTED!\n";
    return xn;
  }

  double hpf = alpha0_ * (xn - rho_ * z11_ - z12_);

  double bpf = alpha_ * hpf + z11_;

  if (nlp_) bpf = tanh(saturation_ * bpf);

  double lpf = alpha_ * bpf + z12_;

  // double R = 1.0 / (2.0 * q_);

  // double bsf = xn - 2.0 * R * bpf; // hmm, this is unused - mistake?

  double semBSF = aux_control_ * hpf + (1.0 - aux_control_) * lpf;

  z11_ = alpha_ * hpf + bpf;
  z12_ = alpha_ * bpf + lpf;

  if (filter_type_ == LPF2)
    return lpf;
  else if (filter_type_ == HPF2)
    return hpf;
  else if (filter_type_ == BPF2)
    return bpf;
  else if (filter_type_ == BSF2)
    return semBSF;

  // Should never reach here due to check above, but needed for compiler
  return xn;
}
