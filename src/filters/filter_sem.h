#pragma once

#include "defjams.h"
#include "filter.h"

class FilterSem : public Filter {
 public:
  FilterSem();
  ~FilterSem() override = default;

  void SetQControl(double qcontrol) override;
  void Reset() override;
  void Update() override;
  double DoFilter(double xn) override;

 private:
  double alpha_{1.0};
  double alpha0_{1.0};
  double rho_{1.0};
  double z11_{0.0};
  double z12_{0.0};
};
