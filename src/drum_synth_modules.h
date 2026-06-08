#pragma once

#include <qblimited_oscillator.h>

#include <array>
#include <memory>
#include <vector>

#include "dca.h"
#include "defjams.h"
#include "distortion.h"
#include "envelope_generator.h"
#include "filters/filter_ckthreefive.h"
#include "filters/filter_moogladder.h"
#include "filters/filter_sem.h"
#include "fx/stereodelay.h"
#include "lfo.h"
#include "qblimited_oscillator.h"

namespace SBAudio {

// SquareOscillatorBank used in the HiHat module below

const int kNumOscillators{6};
const std::array<float, kNumOscillators> kOscFrequencies{263, 400, 421,
                                                         474, 587, 845};
// const std::array<float, kNumOscillators> kOscFrequencies{80,    120,   166.4,
//                                                          217.2, 271.6,
//                                                          328.4};
//
class PulseTrigger {
 public:
  PulseTrigger() = default;
  ~PulseTrigger() = default;

  void Trigger();
  double GenNext();

  double amplitude_{0.6};
  int pulse_counter_{0};
  int pulse_length_{44};  // samples
};

const std::array<bool, kNumOscillators> kDefaultOscConfig{true, true, true,
                                                          true, true, true};
const float kSquareOscAmplitude = 0.4;

class SquareOscillatorBank {
 public:
  SquareOscillatorBank();
  ~SquareOscillatorBank() = default;

  double DoGenerate();

  void Start();
  void Stop();

  void SetAmplitude(double amp);
  bool IsNoteOn();

 private:
  std::vector<std::unique_ptr<QBLimitedOscillator>> oscillators_;
};

class DrumModule {
 public:
  virtual void NoteOn(double vel) = 0;
  virtual void DoRetrigger(
      double vel) = 0;  // Subclass implements actual restart
  virtual StereoVal Generate() = 0;

  // Call this instead of restarting envelopes directly on retrigger
  void RequestRetrigger(double vel) {
    pending_velocity_ = vel;
    if (!pending_retrigger_) {
      pending_retrigger_ = true;
      retrigger_ramp_ = 1.0;  // only reset if not already fading
    }
  }

  // Call this in Generate() to apply fade and check for retrigger completion
  StereoVal ApplyRetriggerFade(StereoVal out) {
    if (pending_retrigger_) {
      out.left *= retrigger_ramp_;
      out.right *= retrigger_ramp_;
      retrigger_ramp_ *= retrigger_fade_factor_;  // Exponential decay
      if (retrigger_ramp_ <= 0.001) {             // -60dB threshold
        retrigger_ramp_ = 0.0;
        pending_retrigger_ = false;
        fadein_active_ = true;
        fadein_ramp_ = 0.0;
        DoRetrigger(pending_velocity_);
      }
    } else if (fadein_active_) {
      out.left *= fadein_ramp_;
      out.right *= fadein_ramp_;
      fadein_ramp_ += fadein_rate_;
      if (fadein_ramp_ >= 1.0) {
        fadein_ramp_ = 1.0;
        fadein_active_ = false;
      }
    }
    return out;
  }

  double velocity_{1};

  EnvelopeGenerator eg_;
  DCA dca_;
  bool use_distortion_{false};
  Distortion distortion_;
  bool use_delay_{false};
  bool note_on_{false};
  std::unique_ptr<StereoDelay> delay_;

  // Retrigger fade-out to prevent clicks
  // Exponential fade: 10ms to reach -60dB (0.001) at 44.1kHz
  // Factor = 0.001^(1/441) ≈ 0.9844
  bool pending_retrigger_{false};
  double pending_velocity_{0};
  double retrigger_ramp_{0};
  static constexpr double retrigger_fade_factor_{0.9844};

  // Fade-in after retrigger (5ms linear ramp)
  bool fadein_active_{false};
  double fadein_ramp_{0};
  static constexpr double fadein_rate_{1.0 / 220.0};  // 5ms at 44.1kHz
};

const double kDefaultKickFrequency = 80;

class BassDrum : public DrumModule {
 public:
  BassDrum();
  virtual ~BassDrum() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  bool hard_sync_{false};
  double frequency_{kDefaultKickFrequency};
  double pitch_osc_range_{2.0};   // Pitch envelope range in semitones
  double pitch_osc_range2_{0.0};  // Second (fast) pitch EG range — 0 = off
  EnvelopeGenerator pitch_eg2_;   // Fast transient pitch envelope

  PulseTrigger click_;

  bool noise_enabled_{false};
  std::unique_ptr<QBLimitedOscillator> noise_;
  EnvelopeGenerator noise_eg_;
  // std::unique_ptr<CKThreeFive> noise_filter_;
  std::unique_ptr<FilterSem> noise_filter_;

  std::unique_ptr<QBLimitedOscillator> osc1_;
  std::unique_ptr<QBLimitedOscillator> osc2_;

  std::unique_ptr<CKThreeFive> out_filter_;

  // Chirp: short exponential frequency sweep mixed into the attack transient
  bool chirp_enabled_{false};
  double chirp_start_freq_{3000.0};
  double chirp_end_freq_{200.0};
  double chirp_decay_ms_{10.0};
  double chirp_amplitude_{0.316};  // -10dB
  std::unique_ptr<QBLimitedOscillator> chirp_osc_;
  EnvelopeGenerator chirp_eg_;
  int chirp_timer_{0};
  int chirp_duration_samples_{441};  // 10ms at 44100
};

const float kHighSnareFreq = 476;
const float kLowSnareFreq = 238;

class SnareDrum : public DrumModule {
 public:
  SnareDrum();
  virtual ~SnareDrum() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  float low_freq_{kLowSnareFreq};
  float high_freq_{kHighSnareFreq};

  std::unique_ptr<QBLimitedOscillator> noise_;
  EnvelopeGenerator noise_eg_;
  std::unique_ptr<CKThreeFive> noise_filter_;

  std::unique_ptr<QBLimitedOscillator> lo_osc_;
  std::unique_ptr<QBLimitedOscillator> hi_osc_;

  EnvelopeGenerator pitch_eg_;
  double pitch_eg_depth_{0.0};  // semitones; 0 = no sweep
};

class HandClap : public DrumModule {
 public:
  HandClap();
  virtual ~HandClap() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  std::unique_ptr<QBLimitedOscillator> noise_;
  EnvelopeGenerator noise_eg_;
  std::unique_ptr<FilterSem> noise_filter_;

  std::unique_ptr<LFO> lfo_;
};

class HiHat : public DrumModule {
 public:
  HiHat();
  virtual ~HiHat() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  void SetAmplitude(double val);

  SquareOscillatorBank osc_bank_;
  // std::unique_ptr<FilterSem> mid_filter_;
  std::unique_ptr<MoogLadder> mid_filter_;
  std::unique_ptr<FilterSem> high_filter_;
  // std::unique_ptr<MoogLadder> high_filter_;
};

class FMDrum : public DrumModule {
 public:
  FMDrum();
  virtual ~FMDrum() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  std::unique_ptr<QBLimitedOscillator> carrier_;

  std::unique_ptr<QBLimitedOscillator> modulator_;
  EnvelopeGenerator modulator_eg_;
  double modulator_freq_ratio_{2};
};

const double kDefaultPitchOscRange = 47;

class Lazer : public DrumModule {
 public:
  Lazer();
  virtual ~Lazer() = default;

  void NoteOn(double vel) override;
  void DoRetrigger(double vel) override;
  StereoVal Generate() override;

  std::unique_ptr<QBLimitedOscillator> osc1_;

  double pitch_osc_range_{kDefaultPitchOscRange};
};

}  // namespace SBAudio
