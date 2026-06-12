#include "drum_synth_modules.h"

#include <cassert>
#include <iostream>

namespace SBAudio {

BassDrum::BassDrum() {
  // TRANSIENT
  noise_ = std::make_unique<QBLimitedOscillator>();
  noise_->m_waveform = NOISE;
  noise_->m_amplitude = 0.6;
  noise_->Update();

  noise_eg_.SetRampMode(true);
  noise_eg_.m_reset_to_zero = true;
  noise_eg_.SetEgMode(DIGITAL);
  noise_eg_.SetAttackTimeMsec(5);
  noise_eg_.SetDecayTimeMsec(207);
  noise_eg_.Update();

  // noise_filter_ = std::make_unique<CKThreeFive>();
  noise_filter_ = std::make_unique<FilterSem>();
  noise_filter_->SetType(LPF2);
  noise_filter_->SetFcControl(10000);
  noise_filter_->SetQControlGUI(5);
  noise_filter_->Update();

  // PITCH
  osc1_ = std::make_unique<QBLimitedOscillator>();
  osc1_->m_waveform = SINE;
  osc1_->m_osc_fo = frequency_;
  osc1_->m_amplitude = 0.9;
  osc1_->Update();

  osc2_ = std::make_unique<QBLimitedOscillator>();
  osc2_->m_waveform = TRI;
  osc2_->m_osc_fo = frequency_;
  osc2_->m_amplitude = 0.9;
  osc2_->Update();

  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(DIGITAL);
  eg_.SetAttackTimeMsec(1);
  eg_.SetDecayTimeMsec(180);
  eg_.SetSustainLevel(0.0);
  eg_.Update();

  pitch_eg2_.SetRampMode(true);
  pitch_eg2_.m_reset_to_zero = true;
  pitch_eg2_.SetEgMode(DIGITAL);
  pitch_eg2_.SetAttackTimeMsec(1);
  pitch_eg2_.SetDecayTimeMsec(10);
  pitch_eg2_.SetSustainLevel(0.0);
  pitch_eg2_.Update();

  mod_osc_ = std::make_unique<QBLimitedOscillator>();
  mod_osc_->m_waveform = SINE;
  mod_osc_->m_amplitude = 1.0;
  mod_osc_->Update();

  mod_eg_.SetRampMode(true);
  mod_eg_.m_reset_to_zero = true;
  mod_eg_.SetEgMode(DIGITAL);
  mod_eg_.SetAttackTimeMsec(1);
  mod_eg_.SetDecayTimeMsec(200);
  mod_eg_.SetSustainLevel(0.0);
  mod_eg_.Update();

  chirp_osc_ = std::make_unique<QBLimitedOscillator>();
  chirp_osc_->m_waveform = SINE;
  chirp_osc_->m_amplitude = 1.0;
  chirp_osc_->Update();

  chirp_eg_.SetRampMode(true);
  chirp_eg_.m_reset_to_zero = true;
  chirp_eg_.SetEgMode(DIGITAL);
  chirp_eg_.SetAttackTimeMsec(0.1);
  chirp_eg_.SetDecayTimeMsec(10.0);
  chirp_eg_.SetSustainLevel(0.0);
  chirp_eg_.Update();

  distortion_.SetParam("threshold", 0.5);
  delay_ = std::make_unique<StereoDelay>();

  out_filter_ = std::make_unique<CKThreeFive>();
  out_filter_->SetType(LPF2);
  out_filter_->SetFcControl(10000);
  out_filter_->SetQControlGUI(1);
  out_filter_->Update();
}

void BassDrum::NoteOn(double vel) {
  if (note_on_ || osc1_->m_note_on || pending_retrigger_ || fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void BassDrum::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  click_.Trigger();

  osc1_->m_osc_fo = frequency_;
  osc1_->StartOscillator();
  osc2_->m_osc_fo = frequency_;
  osc2_->StartOscillator();

  noise_->StartOscillator();
  noise_eg_.StartEg();

  eg_.StartEg();
  pitch_eg2_.StartEg();

  if (mod_enabled_) {
    mod_osc_->m_osc_fo = mod_freq_;
    mod_osc_->StartOscillator();
    mod_eg_.StartEg();
  }

  if (chirp_enabled_) {
    chirp_timer_ = 0;
    chirp_duration_samples_ = (int)(chirp_decay_ms_ * 44100.0 / 1000.0);
    chirp_osc_->m_osc_fo = chirp_start_freq_;
    chirp_osc_->StartOscillator();
    chirp_eg_.StartEg();
  }
}

StereoVal BassDrum::Generate() {
  StereoVal out = {.left = 0, .right = 0};
  if (osc1_->m_note_on || noise_->m_note_on) {
    // Transient
    noise_->Update();
    double noise_eg_out = noise_eg_.DoEnvelope(nullptr);
    double noise_out = noise_->DoOscillate(nullptr) * noise_eg_out * 0.2;
    noise_filter_->Update();
    noise_out = noise_filter_->DoFilter(noise_out);

    // OSCILLATORS

    double biased_eg_out = 0;
    double amp_eg_out = eg_.DoEnvelope(&biased_eg_out);

    double biased_eg2_out = 0;
    pitch_eg2_.DoEnvelope(&biased_eg2_out);

    double eg_osc_mod =
        pitch_osc_range_ * biased_eg_out + pitch_osc_range2_ * biased_eg2_out;

    // FM: modulator oscillator modulates osc1_ frequency linearly
    double fm_lin = 0.0;
    if (mod_enabled_) {
      mod_osc_->Update();
      double mod_eg_out = mod_eg_.DoEnvelope(nullptr);
      fm_lin = mod_osc_->DoOscillate(nullptr) * mod_eg_out * mod_index_;
    }

    osc1_->SetFoModExp(eg_osc_mod);
    osc1_->SetFoModLin(fm_lin);
    osc1_->Update();

    osc2_->SetFoModExp(eg_osc_mod);
    osc2_->Update();

    double osc1_out = osc1_->DoOscillate(nullptr) * amp_eg_out;
    if (hard_sync_ && osc1_->just_wrapped) osc2_->StartOscillator();
    double osc2_out = osc2_->DoOscillate(nullptr) * amp_eg_out;

    double osc_mix = click_.GenNext() + osc1_out + osc2_out;

    if (chirp_enabled_) {
      double ratio = (chirp_duration_samples_ > 0)
                         ? (double)chirp_timer_ / chirp_duration_samples_
                         : 1.0;
      if (ratio > 1.0) ratio = 1.0;
      double chirp_freq =
          chirp_start_freq_ * pow(chirp_end_freq_ / chirp_start_freq_, ratio);
      chirp_osc_->m_osc_fo = chirp_freq;
      chirp_osc_->Update();
      double chirp_eg_out = chirp_eg_.DoEnvelope(nullptr);
      osc_mix +=
          chirp_osc_->DoOscillate(nullptr) * chirp_eg_out * chirp_amplitude_;
      chirp_timer_++;
    }
    if (noise_enabled_) osc_mix += noise_out;

    //// OUTPUT //////////////////////////

    // FILTER ////////////////////
    out_filter_->Update();
    double osc_out = out_filter_->DoFilter(osc_mix);

    double out_left = 0.0;
    double out_right = 0.0;

    dca_.Update();
    dca_.DoDCA(osc_out, osc_out, &out_left, &out_right);

    out = {.left = out_left * velocity_, .right = out_right * velocity_};
    if (use_distortion_) out = distortion_.Process(out);
  }

  // Apply retrigger fade OUTSIDE the generation block so it always runs
  out = ApplyRetriggerFade(out);

  if (eg_.GetState() == OFFF) {
    osc1_->StopOscillator();
    osc2_->StopOscillator();
    eg_.StopEg();
  }

  if (noise_eg_.GetState() == OFFF) {
    noise_->StopOscillator();
    noise_eg_.StopEg();
  }
  if (eg_.GetState() == OFFF && noise_eg_.GetState() == OFFF) note_on_ = false;

  if (use_delay_) out = delay_->Process(out);
  return out;
}

SnareDrum::SnareDrum() {
  // TRANSIENT
  noise_ = std::make_unique<QBLimitedOscillator>();
  noise_->m_waveform = NOISE;
  noise_->m_amplitude = 0.6;
  noise_->Update();

  noise_eg_.SetRampMode(true);
  noise_eg_.m_reset_to_zero = true;
  noise_eg_.SetEgMode(DIGITAL);
  noise_eg_.SetAttackTimeMsec(1);
  noise_eg_.SetDecayTimeMsec(27);
  noise_eg_.Update();

  noise_filter_ = std::make_unique<CKThreeFive>();
  noise_filter_->SetType(HPF2);
  noise_filter_->SetFcControl(1000);
  noise_filter_->SetQControlGUI(1);
  noise_filter_->Update();

  // PITCH
  lo_osc_ = std::make_unique<QBLimitedOscillator>();
  lo_osc_->m_waveform = SINE;
  lo_osc_->m_osc_fo = low_freq_;
  lo_osc_->m_amplitude = 1;
  lo_osc_->Update();

  hi_osc_ = std::make_unique<QBLimitedOscillator>();
  hi_osc_->m_waveform = SINE;
  hi_osc_->m_osc_fo = high_freq_;
  hi_osc_->m_amplitude = 1;
  hi_osc_->Update();

  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(ANALOG);
  eg_.SetAttackTimeMsec(1);
  eg_.SetDecayTimeMsec(100);
  eg_.Update();

  pitch_eg_.SetRampMode(true);
  pitch_eg_.m_reset_to_zero = true;
  pitch_eg_.SetEgMode(DIGITAL);
  pitch_eg_.SetAttackTimeMsec(1);
  pitch_eg_.SetDecayTimeMsec(30);
  pitch_eg_.SetSustainLevel(0.0);
  pitch_eg_.Update();

  distortion_.SetParam("threshold", 0.5);
  delay_ = std::make_unique<StereoDelay>();
}

void SnareDrum::NoteOn(double vel) {
  if (note_on_ || lo_osc_->m_note_on || pending_retrigger_ || fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void SnareDrum::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  noise_->StartOscillator();
  noise_eg_.StartEg();

  lo_osc_->StartOscillator();
  hi_osc_->StartOscillator();

  eg_.StartEg();
  pitch_eg_.StartEg();
}

StereoVal SnareDrum::Generate() {
  StereoVal out = {.left = 0, .right = 0};
  if (lo_osc_->m_note_on) {
    // Transient
    noise_->Update();
    double noise_eg_out = noise_eg_.DoEnvelope(nullptr);
    double noise_out = noise_->DoOscillate(nullptr) * noise_eg_out;
    noise_filter_->Update();
    noise_out = noise_filter_->DoFilter(noise_out);

    // OSCILLATORS

    double pitch_eg_out = pitch_eg_.DoEnvelope(nullptr);
    double pitch_mod = pitch_eg_depth_ * pitch_eg_out;
    lo_osc_->SetFoModExp(pitch_mod);
    hi_osc_->SetFoModExp(pitch_mod);

    lo_osc_->Update();
    hi_osc_->Update();

    double lo_osc_out = lo_osc_->DoOscillate(nullptr);
    double hi_osc_out = hi_osc_->DoOscillate(nullptr);

    double osc_out = 0.5 * lo_osc_out + 0.5 * hi_osc_out + noise_out;

    if (parallel_sat_enabled_)
      osc_out += std::tanh(osc_out * parallel_sat_drive_) * parallel_sat_blend_;

    //// OUTPUT //////////////////////////

    // FILTER ////////////////////

    double out_left = 0.0;
    double out_right = 0.0;

    double amp_eg_out = eg_.DoEnvelope(nullptr);
    double dca_mod_val = amp_eg_out;
    dca_.SetEgMod(dca_mod_val);
    dca_.Update();
    dca_.DoDCA(osc_out, osc_out, &out_left, &out_right);

    out = {.left = out_left * velocity_, .right = out_right * velocity_};

    out = distortion_.Process(out);
  }

  // Apply retrigger fade OUTSIDE the generation block so it always runs
  out = ApplyRetriggerFade(out);

  if (eg_.GetState() == OFFF) {
    lo_osc_->StopOscillator();
    hi_osc_->StopOscillator();
    noise_->StopOscillator();

    eg_.StopEg();
    noise_eg_.StopEg();
    pitch_eg_.StopEg();
    note_on_ = false;
  }

  if (use_delay_) out = delay_->Process(out);
  return out;
}

///// CLAP /////////////////////////////

void HandClap::InitVoice(ClapVoice &v, double attack_ms, double decay_ms,
                         double fc, double lfo_rate) {
  v.noise = std::make_unique<QBLimitedOscillator>();
  v.noise->m_waveform = NOISE;
  v.noise->m_amplitude = 0.6;
  v.noise->Update();

  v.noise_eg.SetRampMode(true);
  v.noise_eg.m_reset_to_zero = true;
  v.noise_eg.SetEgMode(DIGITAL);
  v.noise_eg.SetAttackTimeMsec(attack_ms);
  v.noise_eg.SetDecayTimeMsec(decay_ms);
  v.noise_eg.SetSustainLevel(0.0);
  v.noise_eg.Update();

  v.noise_filter = std::make_unique<FilterSem>();
  v.noise_filter->SetType(BPF2);
  v.noise_filter->SetFcControl(fc);
  v.noise_filter->SetQControlGUI(5);
  v.noise_filter->Update();

  v.lfo = std::make_unique<LFO>();
  v.lfo->m_waveform = usaw;
  v.lfo->m_osc_fo = lfo_rate;
  v.lfo->Update();
}

HandClap::HandClap() {
  InitVoice(voices_[0], 10, 207, 1000, 7);
  InitVoice(voices_[1], 15, 150, 1000, 7);
  InitVoice(voices_[2], 10, 200, 1000, 7);
  InitVoice(voices_[3], 10, 400, 1000, 7);

  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(ANALOG);
  eg_.SetAttackTimeMsec(10);
  eg_.SetDecayTimeMsec(100);
  eg_.SetSustainLevel(0.3);
  eg_.SetReleaseTimeMsec(200);
  eg_.Update();

  distortion_.SetParam("threshold", 0.5);
  delay_ = std::make_unique<StereoDelay>();
}

void HandClap::NoteOn(double vel) {
  if (note_on_ || voices_[0].noise->m_note_on || voices_[2].noise->m_note_on ||
      pending_retrigger_ || fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void HandClap::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  // Voice 1 fires immediately
  voices_[0].noise->StartOscillator();
  voices_[0].lfo->StartOscillator();
  voices_[0].noise_eg.StartEg();

  // Voices 2-4 fire after their respective delays
  voice2_pending_ = true;
  voice2_delay_counter_ = (int)(voice2_delay_ms_ * 44100.0 / 1000.0);
  voice3_pending_ = true;
  voice3_delay_counter_ = (int)(voice3_delay_ms_ * 44100.0 / 1000.0);
  voice4_pending_ = true;
  voice4_delay_counter_ = (int)(voice4_delay_ms_ * 44100.0 / 1000.0);

  eg_.StartEg();
}

static double GenerateVoice(ClapVoice &v) {
  v.noise->Update();
  double noise_eg_out = v.noise_eg.DoEnvelope(nullptr);
  double noise_out = v.noise->DoOscillate(nullptr) * noise_eg_out;
  v.noise_filter->Update();
  double filter_out = v.noise_filter->DoFilter(noise_out);
  v.lfo->Update();
  double lfo_out = v.lfo->DoOscillate(nullptr) * noise_out;
  return lfo_out + filter_out;
}

StereoVal HandClap::Generate() {
  StereoVal out = {.left = 0, .right = 0};

  // Trigger delayed voices after their respective delays
  if (voice2_pending_) {
    if (--voice2_delay_counter_ <= 0) {
      voice2_pending_ = false;
      voices_[1].noise_eg.SetAttackTimeMsec(voice2_attack_ms_);
      voices_[1].noise_eg.SetDecayTimeMsec(voice2_decay_ms_);
      voices_[1].noise_eg.Update();
      voices_[1].noise->StartOscillator();
      voices_[1].lfo->StartOscillator();
      voices_[1].noise_eg.StartEg();
    }
  }
  if (voice3_pending_) {
    if (--voice3_delay_counter_ <= 0) {
      voice3_pending_ = false;
      voices_[2].noise_eg.SetAttackTimeMsec(voice3_attack_ms_);
      voices_[2].noise_eg.SetDecayTimeMsec(voice3_decay_ms_);
      voices_[2].noise_eg.Update();
      voices_[2].noise->StartOscillator();
      voices_[2].lfo->StartOscillator();
      voices_[2].noise_eg.StartEg();
    }
  }
  if (voice4_pending_) {
    if (--voice4_delay_counter_ <= 0) {
      voice4_pending_ = false;
      voices_[3].noise_eg.SetAttackTimeMsec(voice4_attack_ms_);
      voices_[3].noise_eg.SetDecayTimeMsec(voice4_decay_ms_);
      voices_[3].noise_eg.Update();
      voices_[3].noise->StartOscillator();
      voices_[3].lfo->StartOscillator();
      voices_[3].noise_eg.StartEg();
    }
  }

  if (voices_[0].noise->m_note_on || voices_[1].noise->m_note_on ||
      voices_[2].noise->m_note_on || voices_[3].noise->m_note_on) {
    double osc_out = 0.0;
    if (voices_[0].noise->m_note_on) osc_out += GenerateVoice(voices_[0]);
    if (voices_[1].noise->m_note_on)
      osc_out += GenerateVoice(voices_[1]) * voice2_vol_;
    if (voices_[2].noise->m_note_on)
      osc_out += GenerateVoice(voices_[2]) * voice3_vol_;

    // Voice 4 bypasses the shared eg_/DCA so its long tail is not attenuated
    // or cut short by the shared envelope. Its noise_eg is its sole amplitude
    // control.
    double tail_out = 0.0;
    if (voices_[3].noise->m_note_on)
      tail_out = GenerateVoice(voices_[3]) * voice4_vol_;

    double out_left = 0.0;
    double out_right = 0.0;

    double amp_eg_out = eg_.DoEnvelope(nullptr);
    dca_.SetEgMod(amp_eg_out);
    dca_.Update();
    dca_.DoDCA(osc_out, osc_out, &out_left, &out_right);

    out_left += tail_out;
    out_right += tail_out;

    out = {.left = out_left * velocity_, .right = out_right * velocity_};
    out = distortion_.Process(out);
  }

  out = ApplyRetriggerFade(out);

  if (eg_.GetState() == OFFF) {
    for (int i = 0; i < 4; ++i) {
      voices_[i].noise->StopOscillator();
      voices_[i].lfo->StopOscillator();
      voices_[i].noise_eg.StopEg();
    }
    eg_.StopEg();
    note_on_ = false;
  }

  if (use_delay_) out = delay_->Process(out);
  return out;
}

SquareOscillatorBank::SquareOscillatorBank() {
  for (const auto &f : kOscFrequencies) {
    auto osc = std::make_unique<QBLimitedOscillator>();
    osc->m_osc_fo = f;
    osc->m_waveform = SQUARE;
    osc->m_amplitude = kSquareOscAmplitude;
    oscillators_.push_back(std::move(osc));
  }
}

void SquareOscillatorBank::Start() {
  for (const auto &o : oscillators_) {
    o->StartOscillator();
  }
}

void SquareOscillatorBank::Stop() {
  for (const auto &o : oscillators_) {
    o->StopOscillator();
  }
}
bool SquareOscillatorBank::IsNoteOn() {
  assert(oscillators_.size() > 0);
  return oscillators_[0]->m_note_on;
}
void SquareOscillatorBank::SetAmplitude(double amp) {
  for (int i = 0; i < kNumOscillators; i++) {
    oscillators_[i]->m_amplitude = amp;
    oscillators_[i]->Update();
  }
}

double SquareOscillatorBank::DoGenerate() {
  double out = 0;
  for (int i = 0; i < kNumOscillators; i++) {
    if (kDefaultOscConfig[i]) {
      oscillators_[i]->Update();
      out += oscillators_[i]->DoOscillate(nullptr);
    }
  }
  return out;
}

HiHat::HiHat() {
  // mid_filter_ = std::make_unique<FilterSem>();
  mid_filter_ = std::make_unique<MoogLadder>();
  mid_filter_->SetType(BPF2);
  mid_filter_->SetFcControl(10000);
  mid_filter_->SetQControlGUI(1);
  mid_filter_->Update();

  high_filter_ = std::make_unique<FilterSem>();
  // ahigh_filter_ = std::make_unique<MoogLadder>();
  high_filter_->SetType(HPF2);
  high_filter_->SetFcControl(7000);
  high_filter_->SetQControlGUI(1);
  high_filter_->Update();

  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(ANALOG);
  eg_.SetAttackTimeMsec(20);
  eg_.SetDecayTimeMsec(10);
  eg_.SetSustainLevel(0.3);
  eg_.SetReleaseTimeMsec(270);
  eg_.Update();

  distortion_.SetParam("threshold", 0.5);
  delay_ = std::make_unique<StereoDelay>();
}

void HiHat::SetAmplitude(double val) {
  osc_bank_.SetAmplitude(val);
}

void HiHat::NoteOn(double vel) {
  if (note_on_ || osc_bank_.IsNoteOn() || pending_retrigger_ ||
      fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void HiHat::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  osc_bank_.Start();
  eg_.StartEg();
}

StereoVal HiHat::Generate() {
  StereoVal out = {.left = 0, .right = 0};
  if (osc_bank_.IsNoteOn()) {
    double square_out = osc_bank_.DoGenerate();
    // std::cout << "SUQOUT:" << square_out << std::endl;
    mid_filter_->Update();
    double mid_out = mid_filter_->DoFilter(square_out);
    // std::cout << "MIDSUQOUT:" << mid_out << std::endl;

    high_filter_->Update();
    double hi_out = high_filter_->DoFilter(mid_out);
    // std::cout << "HIHGUQOUT:" << hi_out << std::endl;

    double eg_out = eg_.DoEnvelope(nullptr);

    double out_left = 0.0;
    double out_right = 0.0;

    dca_.SetEgMod(eg_out);
    dca_.Update();
    dca_.DoDCA(hi_out, hi_out, &out_left, &out_right);
    // dca_.DoDCA(mid_out, mid_out, &out_left, &out_right);

    // std::cout << "OUT:" << out_left << " VEL<OCL:" << velocity_ << std::endl;
    out = {.left = out_left * velocity_, .right = out_right * velocity_};

    out = distortion_.Process(out);
  }

  // Apply retrigger fade OUTSIDE the generation block so it always runs
  out = ApplyRetriggerFade(out);

  if (eg_.GetState() == OFFF) {
    osc_bank_.Stop();
    eg_.StopEg();
    note_on_ = false;
  }

  if (use_delay_) out = delay_->Process(out);
  return out;
}

void PulseTrigger::Trigger() {
  pulse_counter_ = 0;
}

double PulseTrigger::GenNext() {
  if (pulse_counter_ < pulse_length_) {
    pulse_counter_++;
    return 1.0 * amplitude_;
  }
  return 0;
}

///// FM DRUM /////////////////////////////

FMDrum::FMDrum() {
  carrier_ = std::make_unique<QBLimitedOscillator>();
  carrier_->m_waveform = SINE;
  carrier_->m_osc_fo = 220;
  carrier_->m_amplitude = 1;
  carrier_->Update();

  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(DIGITAL);
  eg_.SetAttackTimeMsec(10);
  eg_.SetDecayTimeMsec(207);
  eg_.Update();

  modulator_ = std::make_unique<QBLimitedOscillator>();
  modulator_->m_waveform = SINE;
  modulator_->m_osc_fo = carrier_->m_osc_fo * modulator_freq_ratio_;
  modulator_->m_amplitude = 0.6;
  modulator_->Update();

  modulator_eg_.SetRampMode(true);
  modulator_eg_.m_reset_to_zero = true;
  modulator_eg_.SetEgMode(DIGITAL);
  modulator_eg_.SetAttackTimeMsec(8);
  modulator_eg_.SetDecayTimeMsec(180);
  modulator_eg_.Update();

  distortion_.SetParam("threshold", 0.5);
  delay_ = std::make_unique<StereoDelay>();
}

void FMDrum::NoteOn(double vel) {
  if (note_on_ || carrier_->m_note_on || pending_retrigger_ || fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void FMDrum::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  carrier_->StartOscillator();
  eg_.StartEg();

  modulator_->StartOscillator();
  modulator_eg_.StartEg();
}

StereoVal FMDrum::Generate() {
  StereoVal out = {.left = 0, .right = 0};

  if (carrier_->m_note_on) {
    modulator_->Update();
    double mod_eg_out = modulator_eg_.DoEnvelope(nullptr);
    double mod_out =
        modulator_->DoOscillate(nullptr) * mod_eg_out * modulator_->m_amplitude;

    carrier_->SetPhaseMod(mod_out);
    carrier_->Update();
    double carrier_eg_out = eg_.DoEnvelope(nullptr);
    double carrier_out = carrier_->DoOscillate(nullptr) * carrier_eg_out;

    //// OUTPUT //////////////////////////

    double out_left = 0.0;
    double out_right = 0.0;

    dca_.Update();
    dca_.DoDCA(carrier_out, carrier_out, &out_left, &out_right);

    out = {.left = out_left * velocity_, .right = out_right * velocity_};

    out = distortion_.Process(out);
  }

  // Apply retrigger fade OUTSIDE the generation block so it always runs
  out = ApplyRetriggerFade(out);

  if (modulator_eg_.GetState() == OFFF) {
    modulator_->StopOscillator();
    modulator_eg_.StopEg();
  }

  if (eg_.GetState() == OFFF) {
    carrier_->StopOscillator();
    eg_.StopEg();
    note_on_ = false;
  }

  if (use_delay_) out = delay_->Process(out);
  return out;
}

///// LAZER  /////////////////////////////

Lazer::Lazer() {
  eg_.SetRampMode(true);
  eg_.m_reset_to_zero = true;
  eg_.SetEgMode(DIGITAL);
  eg_.SetAttackTimeMsec(10);
  eg_.SetDecayTimeMsec(180);
  eg_.Update();

  osc1_ = std::make_unique<QBLimitedOscillator>();
  osc1_->m_waveform = SINE;
  osc1_->m_osc_fo = 220;
  osc1_->m_amplitude = 1;
  osc1_->Update();
}

void Lazer::NoteOn(double vel) {
  if (note_on_ || osc1_->m_note_on || pending_retrigger_ || fadein_active_) {
    RequestRetrigger(vel);
  } else {
    velocity_ = vel;
    note_on_ = true;
    DoRetrigger(vel);
  }
}

void Lazer::DoRetrigger(double vel) {
  velocity_ = vel;
  note_on_ = true;

  osc1_->StartOscillator();
  eg_.StartEg();
}

StereoVal Lazer::Generate() {
  StereoVal out = {.left = 0, .right = 0};
  if (osc1_->m_note_on) {
    double biased_eg_out = 0;
    double amp_eg_out = eg_.DoEnvelope(&biased_eg_out);

    double eg_osc_mod = pitch_osc_range_ * biased_eg_out;
    osc1_->SetFoModExp(eg_osc_mod);
    osc1_->Update();

    double osc_out = osc1_->DoOscillate(nullptr) * amp_eg_out;

    //// OUTPUT //////////////////////////

    double out_left = 0.0;
    double out_right = 0.0;

    dca_.Update();
    dca_.DoDCA(osc_out, osc_out, &out_left, &out_right);

    out = {.left = out_left * velocity_, .right = out_right * velocity_};
  }

  // Apply retrigger fade OUTSIDE the generation block so it always runs
  out = ApplyRetriggerFade(out);

  if (eg_.GetState() == OFFF) {
    osc1_->StopOscillator();
    eg_.StopEg();
    note_on_ = false;
  }

  if (use_delay_) out = delay_->Process(out);
  return out;
}
}  // namespace SBAudio
