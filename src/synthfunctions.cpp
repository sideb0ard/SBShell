#include <synthfunctions.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "fmsynth.h"
#include "minisynth.h"

namespace {

// Read a named preset from a flat JSON file into a map<string,double>.
// Handles both JSON number and boolean values.
std::map<std::string, double> ReadPresetFromJson(const char *path,
                                                 const std::string &name) {
  std::map<std::string, double> result;
  std::ifstream f(path);
  if (!f.is_open()) return result;
  try {
    nlohmann::json root;
    f >> root;
    if (!root.contains(name)) return result;
    for (auto &[k, v] : root[name].items()) {
      if (v.is_number())
        result[k] = v.get<double>();
      else if (v.is_boolean())
        result[k] = v.get<bool>() ? 1.0 : 0.0;
    }
  } catch (...) {
  }
  return result;
}

}  // namespace

double midi_to_pan_value(unsigned int midi_val) {
  // see MMA DLS Level 2 Spec; controls are asymmetrical
  if (midi_val == 64)
    return 0.0;
  else if (midi_val <= 1)  // 0 or 1
    return -1.0;

  return 2.0 * (double)midi_val / 127.0 - 1.0;
}

double mma_midi_to_atten_dB(unsigned int midi_val) {
  if (midi_val == 0) return -96.0;  // dB floor

  return 20.0 * log10((127.0 * 127.0) / ((float)midi_val * (float)midi_val));
}

double midi_to_bipolar(unsigned int midi_val) {
  return 2.0 * (double)midi_val / 127.0 - 1.0;
}

double calculate_dx_amp(double dx_level) {
  // algo all from Will Pirkle
  double dx_amp = 0.0;
  if (dx_level != 0.0) {
    dx_amp = dx_level;
    dx_amp -= 99.0;
    dx_amp /= 1.32;

    dx_amp = (pow(10.0, dx_amp / 20.0));
  }
  return dx_amp;
}

std::map<std::string, double> GetPreset(int id, std::string preset_name) {
  std::map<std::string, double> preset_vals;

  if (preset_name.empty()) {
    printf(
        "Play tha game, pal, need a name to LOAD yer synth settings "
        "with\n");
    return preset_vals;
  }

  // Drums use JSON (LoadPreset handles it directly); no map needed here.
  if (id == DRUMSYNTH_TYPE) return preset_vals;

  if (id == FMSYNTH_TYPE)
    return ReadPresetFromJson(FM_PRESET_FILENAME_JSON, preset_name);
  if (id == MINISYNTH_TYPE)
    return ReadPresetFromJson(MOOG_PRESET_FILENAME_JSON, preset_name);

  return preset_vals;
}
