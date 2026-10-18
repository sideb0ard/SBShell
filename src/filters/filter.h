#pragma once

#include "defjams.h"
#include "modmatrix.h"
#include "synthfunctions.h"

// 46.88.. = semitones between frequencies (80, 18000.0) / 2
// taken from Will Pirkle book 'designing software synths..'
#define FILTER_FC_MOD_RANGE 46.881879936465680
#define FILTER_FC_MIN 80         // 80Hz
#define FILTER_FC_MAX 18000      // 18 kHz
#define FILTER_FC_DEFAULT 10000  // 10kHz
#define FILTER_Q_DEFAULT 0.707   // butterworth (noidea!)
#define FILTER_Q_MOD_RANGE 10    // dunno if this will work!
#define FILTER_TYPE_DEFAULT LPF4

class Filter {
 public:
  Filter() = default;
  Filter(unsigned int ftype) : filter_type_{ftype} {};
  virtual ~Filter() = default;

 public:
  ModulationMatrix *modmatrix_{nullptr};
  GlobalFilterParams *global_filter_params_{nullptr};

  // sources
  unsigned mod_source_fc_{DEST_NONE};
  unsigned mod_source_fc_control_{DEST_NONE};

  // GUI controls
  double fc_control_{FILTER_FC_DEFAULT};  // filter cut-off
  double q_control_{1};                   // 'qualvity factor' 1-10
  double aux_control_{0};  // a spare control, used in SEM and ladder filters

  unsigned nlp_{0};       // Non Linear Processing on/off switch
  double saturation_{1};  // used in NLP

  unsigned filter_type_{LPF1};

  double fc_{FILTER_FC_DEFAULT};         // current filter cut-off val
  double fc_smooth_{FILTER_FC_DEFAULT};  // one-pole smoothed fc (prevents
                                         // crackling on key changes)
  double q_{FILTER_Q_DEFAULT};           // current q value
  double fc_mod_{0};                     // frequency cutoff modulation input

 public:
  void SetFcMod(double d);
  void SetQControlGUI(double d);
  virtual void SetQControl(double d) = 0;
  virtual void SetFcControl(double val);
  virtual void Update();
  virtual void Reset() = 0;
  virtual double DoFilter(double xn) = 0;

  void SetType(unsigned int type);
  void InitGlobalParameters(GlobalFilterParams *params);
};
