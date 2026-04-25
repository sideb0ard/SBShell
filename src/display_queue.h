#pragma once

#include <string>

enum class DisplayType { BAR, PLOT };

struct ScheduledDisplayItem {
  int target_tick{0};
  double val{0};
  int width{40};
  std::string label{};
  int row{-1};
  DisplayType type{DisplayType::BAR};
};
