#include <defjams.h>
#include <filter.h>
#include <filter_moogladder.h>
#include <filter_onepole.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.h>

#include <iostream>

MoogLadder::MoogLadder() {
  lpf1_.filter_type_ = LPF1;
  lpf2_.filter_type_ = LPF1;
  lpf3_.filter_type_ = LPF1;
  lpf4_.filter_type_ = LPF1;

  filter_type_ = LPF4;  // default

  k_ = 0.0;
  alpha_0_ = 1.0;
  a_ = 0.0;
  b_ = 0.0;
  c_ = 0.0;
  d_ = 0.0;
  e_ = 0.0;

  q_control_ = 8.0;  // Q is 1 to 10
}

void MoogLadder::SetQControl(double qcontrol) {
  k_ = (4.0) * (qcontrol - 1.0) / (10.0 - 1.0);

  // k = 4 is the self-oscillation point. With NLP on, the saturator keeps it
  // in check; without it the output runs 20-200x hot, so stay just below.
  static constexpr double kMaxLinearK = 3.9;
  if (!nlp_ && k_ > kMaxLinearK) k_ = kMaxLinearK;
}

void MoogLadder::Update() {
  Filter::Update();

  double wd = 2.0 * M_PI * fc_;
  static double T = 1.0 / SAMPLE_RATE;
  double wa = (2.0 / T) * tan(wd * T / 2.0);
  double g = wa * T / 2.0;

  double G = g / (1.0 + g);

  lpf1_.SetAlpha(G);
  lpf2_.SetAlpha(G);
  lpf3_.SetAlpha(G);
  lpf4_.SetAlpha(G);

  lpf1_.SetBeta(G * G * G / (1.0 + g));
  lpf2_.SetBeta(G * G / (1.0 + g));
  lpf3_.SetBeta(G / (1.0 + g));
  lpf4_.SetBeta(1.0 / (1.0 + g));

  gamma_ = G * G * G * G;
  alpha_0_ = 1.0 / (1.0 + k_ * gamma_);

  switch (filter_type_) {
    case LPF4:
      a_ = 0.0;
      b_ = 0.0;
      c_ = 0.0;
      d_ = 0.0;
      e_ = 1.0;
      break;
    case LPF2:
      a_ = 0.0;
      b_ = 0.0;
      c_ = 1.0;
      d_ = 0.0;
      e_ = 0.0;
      break;
    case BPF4:
      a_ = 0.0;
      b_ = 0.0;
      c_ = 4.0;
      d_ = -8.0;
      e_ = 4.0;
      break;
    case BPF2:
      a_ = 0.0;
      b_ = 2.0;
      c_ = -2.0;
      d_ = 0.0;
      e_ = 0.0;
      break;
    case HPF4:
      a_ = 1.0;
      b_ = -4.0;
      c_ = 6.0;
      d_ = -4.0;
      e_ = -1.0;
      break;
    case HPF2:
      a_ = 1.0;
      b_ = -2.0;
      c_ = 1.0;
      d_ = 0.0;
      e_ = 0.0;
      break;
    default:  // LPF4
      a_ = 0.0;
      b_ = 0.0;
      c_ = 0.0;
      d_ = 0.0;
      e_ = 1.0;
  }
}

void MoogLadder::Reset() {
  lpf1_.Reset();
  lpf2_.Reset();
  lpf3_.Reset();
  lpf4_.Reset();
}

double MoogLadder::DoFilter(double xn) {
  if (filter_type_ == BSF2 || filter_type_ == LPF1 || filter_type_ == HPF1) {
    return xn;
  }

  double sigma = lpf1_.GetFeedbackOutput() + lpf2_.GetFeedbackOutput() +
                 lpf3_.GetFeedbackOutput() + lpf4_.GetFeedbackOutput();

  xn *= 1.0 + aux_control_ * k_;

  double u = (xn - k_ * sigma) * alpha_0_;
  if (nlp_) {
    u = fasttanh(saturation_ * u);
  }

  double LP1 = lpf1_.DoFilter(u);
  double LP2 = lpf2_.DoFilter(LP1);
  double LP3 = lpf3_.DoFilter(LP2);
  double LP4 = lpf4_.DoFilter(LP3);

  return a_ * u + b_ * LP1 + c_ * LP2 + d_ * LP3 + e_ * LP4;
}
