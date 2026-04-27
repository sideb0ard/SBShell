#pragma once

#include <defjams.h>
#include <envelope_generator.h>
#include <soundgenerator.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "filebuffer.h"
#include "filters/filter_moogladder.h"

namespace SBAudio {

constexpr int kSBSynthNumVoices = 8;

// One voice: a read position into the shared buffer + its own envelope.
struct SBSynthVoice {
  bool active{false};
  int note{-1};
  int velocity{100};
  double read_pos{0.0};
  double increment{1.0};
  EnvelopeGenerator eg;
  EnvelopeGenerator morph_eg;  // per-voice timbral sweep
};

// SBSynth — dual-mode polyphonic synth
//
// WAVETABLE mode (default):
//   Loads any audio file as a waveform and cycles through it at the
//   frequency corresponding to each MIDI note.  Works best with
//   single-cycle waveforms but will work with anything.
//
// SAMPLE mode:
//   Plays a sample at different pitches relative to a configurable root
//   note.  Classic rompler behaviour — load a vocal stab, a chord, or any
//   one-shot and play it chromatically.
//
// Usage:
//   let s = sbsynth();
//   add_buf(s, "perc/vocal.wav");
//   set s:mode 1;        // 0=wavetable (default), 1=sample
//   set s:root 60;       // root note for sample mode (default C4)
//   set s:loop 0;        // 0=one-shot, 1=loop
//   set s:attack  10;    // ms
//   set s:decay   200;   // ms
//   set s:sustain 0.7;
//   set s:release 300;   // ms
//   set s:cutoff  8000;  // filter cutoff Hz
//   set s:q       0.7;   // filter resonance
//   note_on(s, 60);
class SBSynth : public SoundGenerator {
 public:
  enum class Mode { WAVETABLE, SAMPLE };

  SBSynth();
  virtual ~SBSynth() = default;

  StereoVal GenNext(mixer_timing_info tinfo) override;
  std::string Info() override;
  std::string Status() override;
  void SetParam(std::string name, double val) override;
  void Start() override;
  void Stop() override;
  void NoteOn(midi_event ev) override;
  void NoteOff(midi_event ev) override;
  void AllNotesOff() override;
  void AddBuffer(std::unique_ptr<FileBuffer> fb) override;

 private:
  Mode mode_{Mode::WAVETABLE};

  std::vector<std::unique_ptr<FileBuffer>> file_buffers_;

  std::array<SBSynthVoice, kSBSynthNumVoices> voices_{};

  // ADSR params shared across all voices (applied on NoteOn)
  double attack_ms_{10.0};
  double decay_ms_{100.0};
  double sustain_lvl_{0.8};
  double release_ms_{200.0};

  // Global Moog ladder filter on the mixed output
  MoogLadder filter_;
  double filter_cutoff_{18000.0};
  double filter_q_{1.0};  // SetQControlGUI range is 1..10

  // Sample mode: MIDI note that plays the sample at 1x speed
  int root_note_{60};

  // Wavetable mode: morph position across loaded buffers (0.0 = first, 1.0 =
  // last)
  double morph_{0.0};

  // Per-voice morph envelope: sweeps each voice's morph independently on NoteOn
  double morph_env_amt_{0.0};  // depth: 0=off, 1=full sweep added to morph_
  double morph_attack_ms_{10.0};
  double morph_decay_ms_{500.0};
  double morph_sustain_lvl_{0.0};  // 0 = one-shot sweep, decays to morph_ base
  double morph_release_ms_{200.0};

  // Phase distortion: warps the sampling position within each cycle
  double pd_amt_{0.0};  // 0.0 = none, 1.0 = full
  int pd_type_{0};      // 0 = sine warp, 1 = power curve

  // Frame skipping / scrubbing
  double scatter_{
      0.0};  // randomise note-on start position (0=always 0, 1=fully random)
  double skip_{
      0.0};  // per-sample probability of a random forward position jump

  // Wavetable loops by default; sample is one-shot by default
  bool loop_{true};

  int FindFreeVoice(int note) const;
  double CalcIncrement(int note) const;
  void InitVoice(SBSynthVoice &v, int note, int velocity);
};

}  // namespace SBAudio
