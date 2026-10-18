#pragma once

#include "defjams.h"
#include "filter.h"
#include "filter_onepole.h"

class MoogLadder : public Filter {
 public:
  MoogLadder();
  ~MoogLadder() override = default;

  void SetQControl(double qcontrol) override;
  void Reset() override;
  void Update() override;
  double DoFilter(double xn) override;

 private:
  double k_{0};
  double gamma_{0};
  double alpha_0_{1.0};

  double a_{0};
  double b_{0};
  double c_{0};
  double d_{0};
  double e_{0};

  OnePole lpf1_;
  OnePole lpf2_;
  OnePole lpf3_;
  OnePole lpf4_;
};
