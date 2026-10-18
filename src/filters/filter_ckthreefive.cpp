#include "filter_ckthreefive.h"

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "defjams.h"
#include "filter.h"
#include "filter_onepole.h"

CKThreeFive::CKThreeFive() {
  filter_type_ = LPF2;
}

void CKThreeFive::SetQControl(double qcontrol) {
  // Map Q control (1-10) to feedback coefficient K (0.5-2.0)
  // Higher K = more resonance
  // Starting from 0.5 to keep low-Q bass drum audible while still having good
  // resonance range
  k_ = 0.5 + (2.0 - 0.5) * (qcontrol - 1.0) / 9.0;
}

void CKThreeFive::Update() {
  Filter::Update();

  double wd = 2.0 * M_PI * fc_;
  double T = 1.0 / SAMPLE_RATE;
  double wa = (2.0 / T) * tan(wd * T / 2.0);
  double g = wa * T / 2.0;

  double G = g / (1.0 + g);

  lpf1_.SetAlpha(G);
  lpf2_.SetAlpha(G);
  hpf1_.SetAlpha(G);
  hpf2_.SetAlpha(G);

  alpha0_ = 1.0 / (1.0 - k_ * G + k_ * G * G);

  if (filter_type_ == LPF2) {
    lpf2_.SetBeta((k_ - k_ * G) / (1.0 + g));
    hpf1_.SetBeta(-1.0 / (1.0 + g));
  } else  // HPF
  {
    hpf2_.SetBeta(-1.0 * G / (1.0 + g));
    lpf1_.SetBeta(1.0 / (1.0 + g));
  }
}

void CKThreeFive::Reset() {
  lpf1_.Reset();
  lpf2_.Reset();
  hpf1_.Reset();
  hpf2_.Reset();
}

double CKThreeFive::DoFilter(double xn) {
  if (filter_type_ != LPF2 && filter_type_ != HPF2) return xn;

  double y = 0.0;
  if (filter_type_ == LPF2) {
    double y1 = lpf1_.DoFilter(xn);

    double S35 = hpf1_.GetFeedbackOutput() + lpf2_.GetFeedbackOutput();

    double u = alpha0_ * (y1 + S35);

    if (nlp_) u = tanh(saturation_ * u);

    y = k_ * lpf2_.DoFilter(u);

    hpf1_.DoFilter(y);
  } else  // HPF
  {
    double y1 = hpf1_.DoFilter(xn);

    double S35 = hpf2_.GetFeedbackOutput() + lpf1_.GetFeedbackOutput();
    double u = alpha0_ * (y1 + S35);
    y = k_ * u;

    if (nlp_) y = tanh(saturation_ * y);

    lpf1_.DoFilter(hpf2_.DoFilter(y));
  }

  // Compensate for output gain, but use sqrt to preserve more resonance
  // character This keeps the filter stable while allowing resonance to affect
  // tone
  if (k_ > 0) {
    y *= 1.0 / sqrt(k_);
  }

  return y;
}
