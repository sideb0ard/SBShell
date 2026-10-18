#include "filter.h"

#include <stdlib.h>

#include <iostream>

#include "defjams.h"
#include "utils.h"

void Filter::SetFcControl(double val) {
  if (val >= FILTER_FC_MIN && val <= FILTER_FC_MAX) {
    fc_control_ = val;
    fc_smooth_ = val;  // snap immediately — smoother only applies to real-time
                       // key-tracking changes, not explicit configuration
  } else {
    std::cout << "FC must be between " << FILTER_FC_MIN << " and "
              << FILTER_FC_MAX << std::endl;
  }
}

void Filter::SetQControlGUI(double val) {
  if (val >= 1 && val <= 10) {
    q_control_ = val;
    // Update();
  } else {
    std::cout << "Q must be between " << 1 << " and " << 10 << std::endl;
  }
}

void Filter::SetFcMod(double val) {
  fc_mod_ = val;
}

void Filter::SetType(unsigned int type) {
  if (type < NUM_FILTER_TYPES) {
    filter_type_ = type;
    // Update();
  } else {
    std::cout << "Type must be between 0 and " << NUM_FILTER_TYPES - 1
              << std::endl;
  }
}

void Filter::Update() {
  if (global_filter_params_) {
    aux_control_ = global_filter_params_->aux_control;
    fc_control_ = global_filter_params_->fc_control;
    q_control_ = global_filter_params_->q_control;
    saturation_ = global_filter_params_->saturation;
    filter_type_ = global_filter_params_->filter_type;
    nlp_ = global_filter_params_->nlp;
  }

  if (modmatrix_) {
    fc_mod_ = modmatrix_->destinations[mod_source_fc_];
    if (modmatrix_->destinations[mod_source_fc_control_] > 0)
      fc_control_ = modmatrix_->destinations[mod_source_fc_control_];
  }
  SetQControl(q_control_);

  double fc_target = fc_control_ * pitch_shift_multiplier(fc_mod_);
  if (fc_target > FILTER_FC_MAX) fc_target = FILTER_FC_MAX;
  if (fc_target < FILTER_FC_MIN) fc_target = FILTER_FC_MIN;

  // One-pole smoother: ~10ms at 44100Hz — prevents crackling when filter
  // cutoff jumps abruptly (e.g. high key-tracking on chord changes).
  static constexpr double kFcSmooth = 0.9977;
  fc_smooth_ = kFcSmooth * fc_smooth_ + (1.0 - kFcSmooth) * fc_target;
  fc_ = fc_smooth_;
}

void Filter::InitGlobalParameters(GlobalFilterParams *params) {
  global_filter_params_ = params;
  global_filter_params_->aux_control = aux_control_;
  global_filter_params_->fc_control = fc_control_;
  global_filter_params_->q_control = q_control_;
  global_filter_params_->saturation = saturation_;
  global_filter_params_->filter_type = filter_type_;
  global_filter_params_->nlp = nlp_;
}
