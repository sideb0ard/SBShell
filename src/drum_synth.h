#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "dca.h"
#include "defjams.h"
#include "drum_synth_modules.h"
#include "soundgenerator.h"

// Per-instrument preset files
static const char DRUM_KITS_FILENAME[] = "settings/drum_kits.json";
static const char DRUM_BD_PRESETS_FILENAME[] = "settings/drum_bd_presets.json";
static const char DRUM_SD_PRESETS_FILENAME[] = "settings/drum_sd_presets.json";
static const char DRUM_HH_PRESETS_FILENAME[] = "settings/drum_hh_presets.json";
static const char DRUM_HH2_PRESETS_FILENAME[] =
    "settings/drum_hh2_presets.json";
static const char DRUM_CP_PRESETS_FILENAME[] = "settings/drum_cp_presets.json";
static const char DRUM_FM_PRESETS_FILENAME[] = "settings/drum_fm_presets.json";
static const char DRUM_LZ_PRESETS_FILENAME[] = "settings/drum_lz_presets.json";

namespace SBAudio {

struct BdSettings {
  double vol{1};
  double pan{0};
  double tone{10000};
  double q{1};
  double noise_enabled{false};
  double noise_vol{0.3};
  double ntone{10000};
  double nq{1};
  double decay{180};
  int octave{2};
  int key{40};
  double detune_cents{0};
  bool use_distortion{true};
  double distortion_threshold{0.5};
  bool hard_sync{false};
  bool use_delay{false};
  int delay_mode{0};  // 0 - norm, 1 - tap1, 2 - tap2, 3 - pingpong
  double delay_ms{23};
  double delay_feedback_pct{0};
  double delay_ratio{0};
  double delay_wetmix{0.5};
  bool delay_sync_tempo{true};
  int delay_sync_len{0};  // 0 none, 1 - 1/4, 2 - 8th, 3 - 16th
  double pitch_env_range{2.0};
  double attack{1.0};
  int osc1_waveform{0};  // SINE default
  int osc2_waveform{4};  // TRI default
  double noise_attack{5.0};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    BdSettings, vol, pan, tone, q, noise_enabled, noise_vol, ntone, nq, decay,
    octave, key, detune_cents, use_distortion, distortion_threshold, hard_sync,
    use_delay, delay_mode, delay_ms, delay_feedback_pct, delay_ratio,
    delay_wetmix, delay_sync_tempo, delay_sync_len, pitch_env_range, attack,
    osc1_waveform, osc2_waveform, noise_attack)

struct SdSettings {
  double vol{1};
  double pan{0};
  double noise_vol{0.5};
  double noise_decay{22};
  double tone{1000};
  double decay{50};
  int octave{3};
  int key{7};
  int hi_osc_waveform{0};
  int lo_osc_waveform{0};
  double distortion_threshold{0.5};
  bool use_delay{false};
  int delay_mode{0};
  double delay_ms{23};
  double delay_feedback_pct{0};
  double delay_ratio{0};
  double delay_wetmix{0.5};
  bool delay_sync_tempo{true};
  int delay_sync_len{0};
  double attack{1.0};
  double noise_attack{1.0};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    SdSettings, vol, pan, noise_vol, noise_decay, tone, decay, octave, key,
    hi_osc_waveform, lo_osc_waveform, distortion_threshold, use_delay,
    delay_mode, delay_ms, delay_feedback_pct, delay_ratio, delay_wetmix,
    delay_sync_tempo, delay_sync_len, attack, noise_attack)

struct HhSettings {
  double vol{1};
  double pan{0};
  double sqamp{0.5};
  double attack{20};
  double decay{10};
  double midf{10000};
  double hif{6000};
  double hif_q{1};
  double distortion_threshold{0.5};
  bool use_delay{false};
  int delay_mode{0};
  double delay_ms{23};
  double delay_feedback_pct{0};
  double delay_ratio{0};
  double delay_wetmix{0.5};
  bool delay_sync_tempo{true};
  int delay_sync_len{0};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    HhSettings, vol, pan, sqamp, attack, decay, midf, hif, hif_q,
    distortion_threshold, use_delay, delay_mode, delay_ms, delay_feedback_pct,
    delay_ratio, delay_wetmix, delay_sync_tempo, delay_sync_len)

struct CpSettings {
  double vol{1};
  double pan{0};
  double nvol{0.6};
  double nattack{10};
  double ndecay{207};
  double tone{1000};
  double fq{5};
  double eg_attack{10};
  double eg_decay{100};
  double eg_sustain{0.3};
  double eg_release{100};
  int lfo_type{usaw};
  double lfo_rate{5};
  double distortion_threshold{0.5};
  bool use_delay{false};
  int delay_mode{0};
  double delay_ms{23};
  double delay_feedback_pct{0};
  double delay_ratio{0};
  double delay_wetmix{0.5};
  bool delay_sync_tempo{true};
  int delay_sync_len{0};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    CpSettings, vol, pan, nvol, nattack, ndecay, tone, fq, eg_attack, eg_decay,
    eg_sustain, eg_release, lfo_type, lfo_rate, distortion_threshold, use_delay,
    delay_mode, delay_ms, delay_feedback_pct, delay_ratio, delay_wetmix,
    delay_sync_tempo, delay_sync_len)

struct FmDrumSettings {
  double vol{0.4};
  double pan{0};
  double carrier_freq{43};
  double carrier_eg_attack{10};
  double carrier_eg_decay{1};
  double carrier_eg_sustain{0.2};
  double carrier_eg_release{10};
  double modulator_freq_ratio{13};
  double modulator_eg_attack{15};
  double modulator_eg_decay{15};
  double modulator_eg_sustain{0.5};
  double modulator_eg_release{150};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    FmDrumSettings, vol, pan, carrier_freq, carrier_eg_attack, carrier_eg_decay,
    carrier_eg_sustain, carrier_eg_release, modulator_freq_ratio,
    modulator_eg_attack, modulator_eg_decay, modulator_eg_sustain,
    modulator_eg_release)

struct LazerSettings {
  double vol{0.7};
  double pan{0};
  double freq{220};
  double attack{10};
  double decay{180};
  double osc_range{47};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LazerSettings, vol, pan, freq,
                                                attack, decay, osc_range)

struct DrumSettings {
  std::string name{"Default"};
  double volume{1};

  BdSettings bd;
  SdSettings sd;
  HhSettings hh;
  HhSettings hh2;
  CpSettings cp;
  FmDrumSettings fm1;
  FmDrumSettings fm2;
  FmDrumSettings fm3;
  LazerSettings lz;

  DrumSettings() {
    fm1.pan = -0.1;

    // hh2 (open hat) has different defaults from hh (closed hat)
    hh2.decay = 100;
    hh2.delay_ms = 13;

    // fm2 and fm3 have different defaults from fm1
    fm2.pan = 0.2;
    fm2.carrier_freq = 66;
    fm2.carrier_eg_attack = 3;
    fm2.carrier_eg_decay = 20;
    fm2.carrier_eg_sustain = 0.5;
    fm2.carrier_eg_release = 100;
    fm2.modulator_freq_ratio = 7.03;
    fm2.modulator_eg_attack = 10;
    fm2.modulator_eg_decay = 70;
    fm2.modulator_eg_sustain = 0.5;
    fm2.modulator_eg_release = 80;

    fm3.pan = 0.1;
    fm3.carrier_freq = 65.4;
    fm3.carrier_eg_attack = 19;
    fm3.carrier_eg_decay = 90;
    fm3.carrier_eg_sustain = 0.2;
    fm3.carrier_eg_release = 90;
    fm3.modulator_freq_ratio = 3.4;
    fm3.modulator_eg_attack = 10;
    fm3.modulator_eg_decay = 10;
    fm3.modulator_eg_sustain = 0.5;
    fm3.modulator_eg_release = 180;
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DrumSettings, name, volume, bd,
                                                sd, hh, hh2, cp, fm1, fm2, fm3,
                                                lz)

class DrumSynth : public SoundGenerator {
 public:
  DrumSynth();
  ~DrumSynth() override = default;

  std::string Info() override;
  std::string Status() override;
  StereoVal GenNext(mixer_timing_info tinfo) override;
  void Start() override;
  void Stop() override;
  void Randomize() override;
  void NoteOn(midi_event ev) override;
  void NoteOff(midi_event ev) override;
  void SetParam(std::string name, double val) override;
  void SetVolume(double v) override;
  void SetPan(double p) override;
  void PrintSettings(DrumSettings settings);
  void LoadPreset(std::string name,
                  std::map<std::string, double> preset_vals) override;
  void Save(std::string name) override;
  void Update();

  void LoadSettings(DrumSettings settings);
  // Per-instrument preset I/O. part =
  // "bd","sd","hh","hh2","cp","fm1"-"fm3","lz"
  void SavePart(std::string preset_name, std::string part);
  bool LoadPart(std::string preset_name, std::string part);

  DrumSettings settings_;

  std::unique_ptr<BassDrum> bd_;
  std::unique_ptr<SnareDrum> sd_;
  std::unique_ptr<HiHat> hh_;
  std::unique_ptr<HiHat> hh2_;
  std::unique_ptr<HandClap> cp_;
  std::unique_ptr<FMDrum> fm1_;
  std::unique_ptr<FMDrum> fm2_;
  std::unique_ptr<FMDrum> fm3_;
  std::unique_ptr<Lazer> lz_;

  DCA dca_;
};

}  // namespace SBAudio
