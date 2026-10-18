#pragma once

#include "defjams.h"
#include "filter.h"

class OnePole : public Filter {
 public:
  OnePole();
  explicit OnePole(unsigned int ftype) : Filter{ftype} {};
  ~OnePole() override = default;

  double DoFilter(double xn) override;
  void Update() override;
  void SetQControl(double /*d*/) override {}
  void SetFeedback(double fb);
  double GetFeedbackOutput();
  void Reset() override;
  void SetFilterType(unsigned int ftype);  // ENUM in filter.h

  // Coefficients set by parent filters (MoogLadder, CKThreeFive).
  void SetAlpha(double alpha) {
    alpha_ = alpha;
  }
  void SetBeta(double beta) {
    beta_ = beta;
  }

 private:
  double alpha_{1.0};
  double beta_{0.0};
  double gamma_{1.0};
  double delta_{0.0};
  double epsilon_{0.0};

  double a0_{1.0};
  double z1_{0.0};
  double feedback_{0.0};
};
