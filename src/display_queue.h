#pragma once

#include <string>
#include <vector>

enum class DisplayType { BAR, PLOT, ARRAY };

struct ScheduledDisplayItem {
  int target_tick{0};
  double val{0};
  int width{40};
  std::string label{};
  int row{-1};
  int bar_pos{
      -1};  // raw at_tick value; enables column-positioned waveform mode
  DisplayType type{DisplayType::BAR};
  // ARRAY: one cell per value (0.0-1.0), drawn as a bar chart `height` rows
  // tall
  std::vector<double> vals{};
  int height{1};
};
