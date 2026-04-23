#pragma once

#include <filter_ckthreefive.h>
#include <fx/fx.h>

#include <memory>

class DjEq : public Fx {
 public:
  DjEq();
  std::string Status() override;
  StereoVal Process(StereoVal input) override;
  void SetParam(std::string name, double val) override;

 private:
  // HPF - rolls off bass (lo knob)
  std::unique_ptr<CKThreeFive> hpf_left_;
  std::unique_ptr<CKThreeFive> hpf_right_;

  // LPF - rolls off treble (hi knob)
  std::unique_ptr<CKThreeFive> lpf_left_;
  std::unique_ptr<CKThreeFive> lpf_right_;

  double lo_freq_{80};     // HPF cutoff - default wide open (bass passes)
  double hi_freq_{18000};  // LPF cutoff - default wide open (treble passes)
};
