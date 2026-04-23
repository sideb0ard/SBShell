#include <fx/djeq.h>

#include <iostream>
#include <sstream>

DjEq::DjEq() {
  type_ = fx_type::DJEQ;

  hpf_left_ = std::make_unique<CKThreeFive>();
  hpf_left_->SetType(HPF2);
  hpf_left_->SetFcControl(lo_freq_);
  hpf_left_->SetQControlGUI(1);

  hpf_right_ = std::make_unique<CKThreeFive>();
  hpf_right_->SetType(HPF2);
  hpf_right_->SetFcControl(lo_freq_);
  hpf_right_->SetQControlGUI(1);

  lpf_left_ = std::make_unique<CKThreeFive>();
  lpf_left_->SetType(LPF2);
  lpf_left_->SetFcControl(hi_freq_);
  lpf_left_->SetQControlGUI(1);

  lpf_right_ = std::make_unique<CKThreeFive>();
  lpf_right_->SetType(LPF2);
  lpf_right_->SetFcControl(hi_freq_);
  lpf_right_->SetQControlGUI(1);

  enabled_ = true;
}

StereoVal DjEq::Process(StereoVal input) {
  hpf_left_->Update();
  hpf_right_->Update();
  lpf_left_->Update();
  lpf_right_->Update();

  StereoVal out;
  out.left = lpf_left_->DoFilter(hpf_left_->DoFilter(input.left));
  out.right = lpf_right_->DoFilter(hpf_right_->DoFilter(input.right));
  return out;
}

void DjEq::SetParam(std::string name, double val) {
  if (name == "lo") {
    lo_freq_ = val;
    hpf_left_->SetFcControl(lo_freq_);
    hpf_right_->SetFcControl(lo_freq_);
  } else if (name == "hi") {
    hi_freq_ = val;
    lpf_left_->SetFcControl(hi_freq_);
    lpf_right_->SetFcControl(hi_freq_);
  }
}

std::string DjEq::Status() {
  std::stringstream ss;
  ss << "djeq lo(hpf):" << lo_freq_ << "hz  hi(lpf):" << hi_freq_ << "hz";
  return ss.str();
}
