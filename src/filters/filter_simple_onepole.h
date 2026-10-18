#pragma once

// Minimal one-pole low-pass ("leaky integrator"):
//
//   y[n] = (1 - g) * x[n] + g * y[n-1]
//
// Set directly by the coefficient g (0..1): 0 = no filtering, closer to 1 =
// darker. Used for the reverb's input and damping filters.
//
// Unlike OnePole (filter_onepole.h), this isn't a Filter subclass and has no
// cutoff/modulation machinery. OnePole is a TPT model of an analog RC filter
// set by cutoff in Hz; this is the cheap textbook version.
class SimpleOnePole {
 public:
  void SetG(double g) {
    g_ = g;
  }
  void Reset() {
    z1_ = 0.0;
  }

  double Process(double xn) {
    z1_ = xn * (1.0 - g_) + g_ * z1_;
    return z1_;
  }

 private:
  double g_{0.0};
  double z1_{0.0};
};
