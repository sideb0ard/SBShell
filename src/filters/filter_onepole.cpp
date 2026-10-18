#include "filter_onepole.h"

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "defjams.h"

OnePole::OnePole() {
  filter_type_ = LPF1;
  Reset();
}

void OnePole::Update() {
  Filter::Update();
  double wd = TWO_PI * fc_;
  double T = 1.0 / SAMPLE_RATE;
  double wa = (2.0 / T) * tan(wd * T / 2.0);
  double g = wa * T / 2.0;

  alpha_ = g / (1.0 + g);
}

void OnePole::SetFeedback(double fb) {
  feedback_ = fb;
}

double OnePole::GetFeedbackOutput() {
  return beta_ * (z1_ + feedback_ * delta_);
}

void OnePole::Reset() {
  z1_ = 0.0;
  feedback_ = 0.0;
}

double OnePole::DoFilter(double xn) {
  if (filter_type_ != LPF1 && filter_type_ != HPF1) {
    std::cerr << "OnePole filter - wrong type!\n";
    return xn;
  }

  xn = xn * gamma_ + feedback_ + epsilon_ * GetFeedbackOutput();

  double vn = (a0_ * xn - z1_) * alpha_;

  double lpf = vn + z1_;

  z1_ = vn + lpf;

  double hpf = xn - lpf;

  if (filter_type_ == LPF1)
    return lpf;
  else if (filter_type_ == HPF1)
    return hpf;

  std::cerr << "SHOULD NOT REACH\n";
  return xn;
}
