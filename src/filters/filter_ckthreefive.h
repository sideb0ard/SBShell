#pragma once

#include "defjams.h"
#include "filter.h"
#include "filter_onepole.h"

class CKThreeFive : public Filter {
 public:
  CKThreeFive();
  ~CKThreeFive() override = default;

  void Update() override;
  void SetQControl(double qcontrol) override;
  void Reset() override;
  double DoFilter(double xn) override;

  double GetK() const {
    return k_;
  }

 private:
  double k_{0.5};
  double alpha0_{0};

  OnePole lpf1_{LPF1};
  OnePole lpf2_{LPF1};
  OnePole hpf1_{HPF1};
  OnePole hpf2_{HPF1};
};
