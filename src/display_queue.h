#pragma once

#include <string>

enum class DisplayType { BAR, PLOT };

struct ScheduledDisplayItem {
  int target_tick{0};
  double val{0};
  int width{40};
  std::string label{};
  int row{-1};
  int bar_pos{
      -1};  // raw at_tick value; enables column-positioned waveform mode
  DisplayType type{DisplayType::BAR};
};
