#include <drum_synth.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "midi_freq_table.h"
#include "utils.h"

namespace {
std::array<std::string, 8> osc_types{"SINE", "SAW1",   "SAW2",  "SAW3",
                                     "TRI",  "SQUARE", "NOISE", "PSNOISE"};
std::string GetOscType(int type) {
  std::string the_type = "dunno";
  if (type < osc_types.size()) return osc_types[type];
  return the_type;
}

}  // namespace

namespace SBAudio {

DrumSynth::DrumSynth() {
  bd_ = std::make_unique<BassDrum>();
  sd_ = std::make_unique<SnareDrum>();
  hh_ = std::make_unique<HiHat>();
  oh_ = std::make_unique<HiHat>();
  cp_ = std::make_unique<HandClap>();
  fm1_ = std::make_unique<FMDrum>();
  fm2_ = std::make_unique<FMDrum>();
  fm3_ = std::make_unique<FMDrum>();
  lz_ = std::make_unique<Lazer>();

  LoadSettings(DrumSettings());
  active = true;
}

StereoVal DrumSynth::GenNext(mixer_timing_info tinfo) {
  StereoVal out = {.left = 0, .right = 0};
  if (!active) return out;

  auto bd_out = bd_->Generate();
  out.left += bd_out.left;
  out.right += bd_out.right;

  auto hh_out = hh_->Generate();
  out.left += hh_out.left;
  out.right += hh_out.right;

  auto oh_out = oh_->Generate();
  out.left += oh_out.left;
  out.right += oh_out.right;

  auto snare_out = sd_->Generate();
  out.left += snare_out.left;
  out.right += snare_out.right;

  auto clap_out = cp_->Generate();
  out.left += clap_out.left;
  out.right += clap_out.right;

  auto fm1_out = fm1_->Generate();
  out.left += fm1_out.left;
  out.right += fm1_out.right;

  auto fm2_out = fm2_->Generate();
  out.left += fm2_out.left;
  out.right += fm2_out.right;

  auto fm3_out = fm3_->Generate();
  out.left += fm3_out.left;
  out.right += fm3_out.right;

  auto lz_out = lz_->Generate();
  out.left += lz_out.left;
  out.right += lz_out.right;

  // Cache per-voice outputs for sidechain tapping (0=bd 1=sd 2=cp 3=hh 4=oh
  // 5=fm1 6=fm2 7=fm3 8=lz)
  voice_cur_val_[0] = bd_out;
  voice_cur_val_[1] = snare_out;
  voice_cur_val_[2] = clap_out;
  voice_cur_val_[3] = hh_out;
  voice_cur_val_[4] = oh_out;
  voice_cur_val_[5] = fm1_out;
  voice_cur_val_[6] = fm2_out;
  voice_cur_val_[7] = fm3_out;
  voice_cur_val_[8] = lz_out;

  out.left = out.left * volume;
  out.right = out.right * volume;

  out = Effector(out);

  return out;
}

void DrumSynth::SetParam(std::string name, double val) {
  if (name == "volume")
    settings_.volume = val;
  else if (name == "bd_vol")
    settings_.bd.vol = val;
  else if (name == "bd_pan")
    settings_.bd.pan = val;
  else if (name == "bd_nvol")
    settings_.bd.noise_vol = val;
  else if (name == "bd_tone")
    settings_.bd.tone = val;
  else if (name == "bd_q")
    settings_.bd.q = val;
  else if (name == "bd_ntone")
    settings_.bd.ntone = val;
  else if (name == "bd_nq")
    settings_.bd.nq = val;
  else if (name == "bd_decay")
    settings_.bd.decay = val;
  else if (name == "bd_freq")
    settings_.bd.frequency = val;
  else if (name == "bd_detune")
    settings_.bd.detune_cents = val;
  else if (name == "bd_hard_sync")
    settings_.bd.hard_sync = val;
  else if (name == "bd_dist_en") {
    settings_.bd.use_distortion = val;
  } else if (name == "bd_dist") {
    settings_.bd.distortion_threshold = val;
  } else if (name == "bd_delay_en")
    settings_.bd.use_delay = val;
  else if (name == "bd_delay_mode")
    settings_.bd.delay_mode = val;
  else if (name == "bd_delay_ms")
    settings_.bd.delay_ms = val;
  else if (name == "bd_delay_feedback_pct")
    settings_.bd.delay_feedback_pct = val;
  else if (name == "bd_delay_ratio")
    settings_.bd.delay_ratio = val;
  else if (name == "bd_delay_wetmix")
    settings_.bd.delay_wetmix = val;
  else if (name == "bd_delay_sync_tempo")
    settings_.bd.delay_sync_tempo = val;
  else if (name == "bd_delay_sync_len")
    settings_.bd.delay_sync_len = val;
  else if (name == "bd_pitch_env_range")
    settings_.bd.pitch_env_range = val;
  else if (name == "bd_pitch_env2_range")
    settings_.bd.pitch_env2_range = val;
  else if (name == "bd_pitch_env2_attack")
    settings_.bd.pitch_env2_attack = val;
  else if (name == "bd_pitch_env2_decay")
    settings_.bd.pitch_env2_decay = val;
  else if (name == "bd_chirp_en")
    settings_.bd.chirp_enabled = val;
  else if (name == "bd_chirp_start")
    settings_.bd.chirp_start_freq = val;
  else if (name == "bd_chirp_end")
    settings_.bd.chirp_end_freq = val;
  else if (name == "bd_chirp_decay")
    settings_.bd.chirp_decay = val;
  else if (name == "bd_chirp_amp")
    settings_.bd.chirp_amp = val;
  else if (name == "bd_mod_en")
    settings_.bd.mod_enabled = val;
  else if (name == "bd_mod_freq")
    settings_.bd.mod_freq = val;
  else if (name == "bd_mod_index")
    settings_.bd.mod_index = val;
  else if (name == "bd_mod_decay")
    settings_.bd.mod_decay = val;
  else if (name == "bd_attack")
    settings_.bd.attack = val;
  else if (name == "bd_osc1_wav")
    settings_.bd.osc1_waveform = val;
  else if (name == "bd_osc2_wav")
    settings_.bd.osc2_waveform = val;
  else if (name == "bd_noise_attack")
    settings_.bd.noise_attack = val;
  else if (name == "bd_noise_en")
    settings_.bd.noise_enabled = val;

  else if (name == "hh_vol")
    settings_.hh.vol = val;
  else if (name == "hh_pan")
    settings_.hh.pan = val;
  else if (name == "hh_attack")
    settings_.hh.attack = val;
  else if (name == "hh_decay")
    settings_.hh.decay = val;
  else if (name == "hh_sqamp")
    settings_.hh.sqamp = val;
  else if (name == "hh_midf")
    settings_.hh.midf = val;
  else if (name == "hh_hif")
    settings_.hh.hif = val;
  else if (name == "hh_hif_q")
    settings_.hh.hif_q = val;
  else if (name == "hh_dist")
    settings_.hh.distortion_threshold = val;
  else if (name == "hh_delay_en")
    settings_.hh.use_delay = val;
  else if (name == "hh_delay_mode")
    settings_.hh.delay_mode = val;
  else if (name == "hh_delay_ms")
    settings_.hh.delay_ms = val;
  else if (name == "hh_delay_feedback_pct")
    settings_.hh.delay_feedback_pct = val;
  else if (name == "hh_delay_ratio")
    settings_.hh.delay_ratio = val;
  else if (name == "hh_delay_wetmix")
    settings_.hh.delay_wetmix = val;
  else if (name == "hh_delay_sync_tempo")
    settings_.hh.delay_sync_tempo = val;
  else if (name == "hh_delay_sync_len")
    settings_.hh.delay_sync_len = val;

  else if (name == "oh_vol")
    settings_.oh.vol = val;
  else if (name == "oh_pan")
    settings_.oh.pan = val;
  else if (name == "oh_attack")
    settings_.oh.attack = val;
  else if (name == "oh_decay")
    settings_.oh.decay = val;
  else if (name == "oh_sqamp")
    settings_.oh.sqamp = val;
  else if (name == "oh_midf")
    settings_.oh.midf = val;
  else if (name == "oh_hif")
    settings_.oh.hif = val;
  else if (name == "oh_hif_q")
    settings_.oh.hif_q = val;
  else if (name == "oh_dist")
    settings_.oh.distortion_threshold = val;
  else if (name == "oh_delay_en")
    settings_.oh.use_delay = val;
  else if (name == "oh_delay_mode")
    settings_.oh.delay_mode = val;
  else if (name == "oh_delay_ms")
    settings_.oh.delay_ms = val;
  else if (name == "oh_delay_feedback_pct")
    settings_.oh.delay_feedback_pct = val;
  else if (name == "oh_delay_ratio")
    settings_.oh.delay_ratio = val;
  else if (name == "oh_delay_wetmix")
    settings_.oh.delay_wetmix = val;
  else if (name == "oh_delay_sync_tempo")
    settings_.oh.delay_sync_tempo = val;
  else if (name == "oh_delay_sync_len")
    settings_.oh.delay_sync_len = val;

  else if (name == "sd_vol")
    settings_.sd.vol = val;
  else if (name == "sd_pan")
    settings_.sd.pan = val;
  else if (name == "sd_nvol")
    settings_.sd.noise_vol = val;
  else if (name == "sd_noise_decay")
    settings_.sd.noise_decay = val;
  else if (name == "sd_tone")
    settings_.sd.tone = val;
  else if (name == "sd_decay")
    settings_.sd.decay = val;
  else if (name == "sd_freq")
    settings_.sd.frequency = val;
  else if (name == "sd_lo_osc_wav")
    settings_.sd.lo_osc_waveform = val;
  else if (name == "sd_hi_osc_wav")
    settings_.sd.hi_osc_waveform = val;
  else if (name == "sd_dist")
    settings_.sd.distortion_threshold = val;
  else if (name == "sd_delay_en")
    settings_.sd.use_delay = val;
  else if (name == "sd_delay_mode")
    settings_.sd.delay_mode = val;
  else if (name == "sd_delay_ms")
    settings_.sd.delay_ms = val;
  else if (name == "sd_delay_feedback_pct")
    settings_.sd.delay_feedback_pct = val;
  else if (name == "sd_delay_ratio")
    settings_.sd.delay_ratio = val;
  else if (name == "sd_delay_wetmix")
    settings_.sd.delay_wetmix = val;
  else if (name == "sd_delay_sync_tempo")
    settings_.sd.delay_sync_tempo = val;
  else if (name == "sd_delay_sync_len")
    settings_.sd.delay_sync_len = val;
  else if (name == "sd_attack")
    settings_.sd.attack = val;
  else if (name == "sd_noise_attack")
    settings_.sd.noise_attack = val;
  else if (name == "sd_pitch_eg_depth")
    settings_.sd.pitch_eg_depth = val;
  else if (name == "sd_pitch_eg_decay")
    settings_.sd.pitch_eg_decay = val;
  else if (name == "sd_hi_ratio")
    settings_.sd.hi_ratio = val;
  else if (name == "sd_psat_en")
    settings_.sd.parallel_sat_enabled = val;
  else if (name == "sd_psat_drive")
    settings_.sd.parallel_sat_drive = val;
  else if (name == "sd_psat_blend")
    settings_.sd.parallel_sat_blend = val;

  else if (name == "cp_vol")
    settings_.cp.vol = val;
  else if (name == "cp_pan")
    settings_.cp.pan = val;
  else if (name == "cp_nvol")
    settings_.cp.nvol = val;
  else if (name == "cp_nattack") {
    settings_.cp.nattack = val;
  } else if (name == "cp_ndecay")
    settings_.cp.ndecay = val;
  else if (name == "cp_tone")
    settings_.cp.tone = val;
  else if (name == "cp_fq")
    settings_.cp.fq = val;
  else if (name == "cp_eg_attack")
    settings_.cp.eg_attack = val;
  else if (name == "cp_eg_decay")
    settings_.cp.eg_decay = val;
  else if (name == "cp_eg_sustain")
    settings_.cp.eg_sustain = val;
  else if (name == "cp_eg_release")
    settings_.cp.eg_release = val;
  else if (name == "cp_lfo_type")
    settings_.cp.lfo_type = val;
  else if (name == "cp_lfo_rate")
    settings_.cp.lfo_rate = val;
  else if (name == "cp_dist")
    settings_.cp.distortion_threshold = val;
  else if (name == "cp_delay_en")
    settings_.cp.use_delay = val;
  else if (name == "cp_delay_mode")
    settings_.cp.delay_mode = val;
  else if (name == "cp_delay_ms")
    settings_.cp.delay_ms = val;
  else if (name == "cp_delay_feedback_pct")
    settings_.cp.delay_feedback_pct = val;
  else if (name == "cp_delay_ratio")
    settings_.cp.delay_ratio = val;
  else if (name == "cp_delay_wetmix")
    settings_.cp.delay_wetmix = val;
  else if (name == "cp_delay_sync_tempo")
    settings_.cp.delay_sync_tempo = val;
  else if (name == "cp_delay_sync_len")
    settings_.cp.delay_sync_len = val;
  else if (name == "cp_v2_delay")
    settings_.cp.v2_delay_ms = val;
  else if (name == "cp_v2_vol")
    settings_.cp.v2_vol = val;
  else if (name == "cp_v2_attack")
    settings_.cp.v2_attack = val;
  else if (name == "cp_v2_decay")
    settings_.cp.v2_decay = val;
  else if (name == "cp_v3_delay")
    settings_.cp.v3_delay_ms = val;
  else if (name == "cp_v3_vol")
    settings_.cp.v3_vol = val;
  else if (name == "cp_v3_attack")
    settings_.cp.v3_attack = val;
  else if (name == "cp_v3_decay")
    settings_.cp.v3_decay = val;
  else if (name == "cp_v4_delay")
    settings_.cp.v4_delay_ms = val;
  else if (name == "cp_v4_vol")
    settings_.cp.v4_vol = val;
  else if (name == "cp_v4_attack")
    settings_.cp.v4_attack = val;
  else if (name == "cp_v4_decay")
    settings_.cp.v4_decay = val;

  else if (name == "fm1_vol")
    settings_.fm1.vol = val;
  else if (name == "fm1_pan")
    settings_.fm1.pan = val;
  else if (name == "fm1_car_freq")
    settings_.fm1.carrier_freq = val;
  else if (name == "fm1_mod_freq_rat")
    settings_.fm1.modulator_freq_ratio = val;
  else if (name == "fm1_car_eg_attack")
    settings_.fm1.carrier_eg_attack = val;
  else if (name == "fm1_car_eg_decay")
    settings_.fm1.carrier_eg_decay = val;
  else if (name == "fm1_car_eg_sustain")
    settings_.fm1.carrier_eg_sustain = val;
  else if (name == "fm1_car_eg_release")
    settings_.fm1.carrier_eg_release = val;
  else if (name == "fm1_mod_eg_attack")
    settings_.fm1.modulator_eg_attack = val;
  else if (name == "fm1_mod_eg_decay")
    settings_.fm1.modulator_eg_decay = val;
  else if (name == "fm1_mod_eg_sustain")
    settings_.fm1.modulator_eg_sustain = val;
  else if (name == "fm1_mod_eg_release")
    settings_.fm1.modulator_eg_release = val;

  else if (name == "fm2_vol")
    settings_.fm2.vol = val;
  else if (name == "fm2_pan")
    settings_.fm2.pan = val;
  else if (name == "fm2_car_freq")
    settings_.fm2.carrier_freq = val;
  else if (name == "fm2_mod_freq_rat")
    settings_.fm2.modulator_freq_ratio = val;
  else if (name == "fm2_car_eg_attack")
    settings_.fm2.carrier_eg_attack = val;
  else if (name == "fm2_car_eg_decay")
    settings_.fm2.carrier_eg_decay = val;
  else if (name == "fm2_car_eg_sustain")
    settings_.fm2.carrier_eg_sustain = val;
  else if (name == "fm2_car_eg_release")
    settings_.fm2.carrier_eg_release = val;
  else if (name == "fm2_mod_eg_attack")
    settings_.fm2.modulator_eg_attack = val;
  else if (name == "fm2_mod_eg_decay")
    settings_.fm2.modulator_eg_decay = val;
  else if (name == "fm2_mod_eg_sustain")
    settings_.fm2.modulator_eg_sustain = val;
  else if (name == "fm2_mod_eg_release")
    settings_.fm2.modulator_eg_release = val;

  else if (name == "fm3_vol")
    settings_.fm3.vol = val;
  else if (name == "fm3_pan")
    settings_.fm3.pan = val;
  else if (name == "fm3_car_freq")
    settings_.fm3.carrier_freq = val;
  else if (name == "fm3_mod_freq_rat")
    settings_.fm3.modulator_freq_ratio = val;
  else if (name == "fm3_car_eg_attack")
    settings_.fm3.carrier_eg_attack = val;
  else if (name == "fm3_car_eg_decay")
    settings_.fm3.carrier_eg_decay = val;
  else if (name == "fm3_car_eg_sustain")
    settings_.fm3.carrier_eg_sustain = val;
  else if (name == "fm3_car_eg_release")
    settings_.fm3.carrier_eg_release = val;
  else if (name == "fm3_mod_eg_attack")
    settings_.fm3.modulator_eg_attack = val;
  else if (name == "fm3_mod_eg_decay")
    settings_.fm3.modulator_eg_decay = val;
  else if (name == "fm3_mod_eg_sustain")
    settings_.fm3.modulator_eg_sustain = val;
  else if (name == "fm3_mod_eg_release")
    settings_.fm3.modulator_eg_release = val;

  else if (name == "lz_vol")
    settings_.lz.vol = val;
  else if (name == "lz_pan")
    settings_.lz.pan = val;
  else if (name == "lz_freq")
    settings_.lz.freq = val;
  else if (name == "lz_attack")
    settings_.lz.attack = val;
  else if (name == "lz_decay")
    settings_.lz.decay = val;
  else if (name == "lz_range")
    settings_.lz.osc_range = val;
  Update();
}

std::string DrumSynth::Info() {
  std::stringstream ss;
  if (!active || volume == 0)
    ss << ANSI_COLOR_RESET;
  else
    ss << COOL_COLOR_PINK2;
  ss << "DrumZynth - " << settings_.name << " - vol:" << volume
     << " pan:" << pan << std::endl;
  ss << COOL_COLOR_YELLOW_MELLOW "     bd(0): bd_vol:" << settings_.bd.vol
     << " bd_pan:" << settings_.bd.pan << " bd_nvol:" << settings_.bd.noise_vol
     << " bd_freq:" << settings_.bd.frequency
     << " bd_detune:" << settings_.bd.detune_cents
     << " bd_hard_sync:" << settings_.bd.hard_sync << std::endl;
  ss << "     bd_tone:" << settings_.bd.tone << " bd_q:" << settings_.bd.q
     << " bd_ntone:" << settings_.bd.ntone << " bd_nq:" << settings_.bd.nq
     << " bd_decay:" << settings_.bd.decay
     << " bd_dist_en:" << settings_.bd.use_distortion
     << " bd_dist:" << settings_.bd.distortion_threshold << std::endl;
  ss << "     bd_delay_mode:" << settings_.bd.delay_mode
     << " bd_delay_en:" << settings_.bd.use_delay
     << " bd_delay_ms:" << settings_.bd.delay_ms
     << " bd_delay_feedback_pct:" << settings_.bd.delay_feedback_pct
     << " bd_delay_ratio:" << settings_.bd.delay_ratio << std::endl;
  ss << "     bd_delay_wetmix:" << settings_.bd.delay_wetmix
     << " bd_delay_sync_tempo:" << settings_.bd.delay_sync_tempo
     << " bd_delay_sync_len:" << settings_.bd.delay_sync_len
     << " bd_pitch_env_range:" << settings_.bd.pitch_env_range << std::endl;
  ss << "     bd_pitch_env2_range:" << settings_.bd.pitch_env2_range
     << " bd_pitch_env2_attack:" << settings_.bd.pitch_env2_attack
     << " bd_pitch_env2_decay:" << settings_.bd.pitch_env2_decay << std::endl;
  ss << "     bd_chirp_en:" << settings_.bd.chirp_enabled
     << " bd_chirp_start:" << settings_.bd.chirp_start_freq
     << " bd_chirp_end:" << settings_.bd.chirp_end_freq
     << " bd_chirp_decay:" << settings_.bd.chirp_decay
     << " bd_chirp_amp:" << settings_.bd.chirp_amp << std::endl;
  ss << "     bd_mod_en:" << settings_.bd.mod_enabled
     << " bd_mod_freq:" << settings_.bd.mod_freq
     << " bd_mod_index:" << settings_.bd.mod_index
     << " bd_mod_decay:" << settings_.bd.mod_decay << std::endl;
  ss << "     bd_attack:" << settings_.bd.attack
     << " bd_osc1_wav:" << GetOscType(settings_.bd.osc1_waveform)
     << " bd_osc2_wav:" << GetOscType(settings_.bd.osc2_waveform)
     << " bd_noise_attack:" << settings_.bd.noise_attack
     << " bd_noise_en:" << settings_.bd.noise_enabled << std::endl;
  ss << COOL_COLOR_ORANGE "     sd(1): sd_vol:" << settings_.sd.vol
     << " sd_pan:" << settings_.sd.pan << " sd_nvol:" << settings_.sd.noise_vol
     << " sd_noise_decay:" << settings_.sd.noise_decay
     << " sd_tone:" << settings_.sd.tone << " sd_decay:" << settings_.sd.decay
     << std::endl;
  ss << "     sd_freq:" << settings_.sd.frequency
     << " sd_lo_osc_wav:" << settings_.sd.lo_osc_waveform
     << " sd_hi_osc_wav:" << settings_.sd.hi_osc_waveform
     << " sd_hi_ratio:" << settings_.sd.hi_ratio
     << " sd_dist:" << settings_.sd.distortion_threshold << std::endl;
  ss << "     sd_delay_mode:" << settings_.sd.delay_mode
     << " sd_delay_en:" << settings_.sd.use_delay
     << " sd_delay_ms:" << settings_.sd.delay_ms
     << " sd_delay_feedback_pct:" << settings_.sd.delay_feedback_pct
     << " sd_delay_ratio:" << settings_.sd.delay_ratio << std::endl;
  ss << "     sd_delay_wetmix:" << settings_.sd.delay_wetmix
     << " sd_delay_sync_tempo:" << settings_.sd.delay_sync_tempo
     << " sd_delay_sync_len:" << settings_.sd.delay_sync_len << std::endl;
  ss << "     sd_attack:" << settings_.sd.attack
     << " sd_noise_attack:" << settings_.sd.noise_attack
     << " sd_pitch_eg_depth:" << settings_.sd.pitch_eg_depth
     << " sd_pitch_eg_decay:" << settings_.sd.pitch_eg_decay << std::endl;
  ss << "     sd_psat_en:" << settings_.sd.parallel_sat_enabled
     << " sd_psat_drive:" << settings_.sd.parallel_sat_drive
     << " sd_psat_blend:" << settings_.sd.parallel_sat_blend << std::endl;
  ss << COOL_COLOR_YELLOW_MELLOW "     cp(2): cp_vol:" << settings_.cp.vol
     << " cp_pan:" << settings_.cp.pan << " cp_nvol:" << settings_.cp.nvol
     << " cp_nattack:" << settings_.cp.nattack
     << " cp_ndecay:" << settings_.cp.ndecay << " cp_tone:" << settings_.cp.tone
     << " cp_fq:" << settings_.cp.fq << std::endl;
  ss << "     cp_eg_attack:" << settings_.cp.eg_attack
     << " cp_eg_decay:" << settings_.cp.eg_decay
     << " cp_eg_sustain:" << settings_.cp.eg_sustain
     << " cp_eg_release:" << settings_.cp.eg_release << std::endl;
  ss << "     cp_lfo_type:" << settings_.cp.lfo_type
     << " cp_lfo_rate:" << settings_.cp.lfo_rate
     << " cp_dist:" << settings_.cp.distortion_threshold << std::endl;
  ss << "     cp_delay_mode:" << settings_.cp.delay_mode
     << " cp_delay_ms:" << settings_.cp.delay_ms
     << " cp_delay_feedback_pct:" << settings_.cp.delay_feedback_pct
     << " cp_delay_ratio:" << settings_.cp.delay_ratio << std::endl;
  ss << "     cp_delay_wetmix:" << settings_.cp.delay_wetmix
     << " cp_delay_sync_tempo:" << settings_.cp.delay_sync_tempo
     << " cp_delay_sync_len:" << settings_.cp.delay_sync_len << std::endl;
  ss << "     cp_v2_delay:" << settings_.cp.v2_delay_ms
     << " cp_v2_vol:" << settings_.cp.v2_vol
     << " cp_v2_attack:" << settings_.cp.v2_attack
     << " cp_v2_decay:" << settings_.cp.v2_decay << std::endl;
  ss << "     cp_v3_delay:" << settings_.cp.v3_delay_ms
     << " cp_v3_vol:" << settings_.cp.v3_vol
     << " cp_v3_attack:" << settings_.cp.v3_attack
     << " cp_v3_decay:" << settings_.cp.v3_decay << std::endl;
  ss << "     cp_v4_delay:" << settings_.cp.v4_delay_ms
     << " cp_v4_vol:" << settings_.cp.v4_vol
     << " cp_v4_attack:" << settings_.cp.v4_attack
     << " cp_v4_decay:" << settings_.cp.v4_decay << " (reverb tail)"
     << std::endl;
  ss << COOL_COLOR_ORANGE "     hh(3): hh_vol:" << settings_.hh.vol
     << " hh_pan:" << settings_.hh.pan << " hh_attack:" << settings_.hh.attack
     << " hh_decay:" << settings_.hh.decay << std::endl;
  ss << "     hh_sqamp:" << settings_.hh.sqamp
     << " hh_midf:" << settings_.hh.midf << " hh_hif:" << settings_.hh.hif
     << " hh_hif_q:" << settings_.hh.hif_q
     << " hh_dist:" << settings_.hh.distortion_threshold << std::endl;
  ss << "     hh_delay_mode:" << settings_.hh.delay_mode
     << " hh_delay_ms:" << settings_.hh.delay_ms
     << " hh_delay_feedback_pct:" << settings_.hh.delay_feedback_pct
     << " hh_delay_ratio:" << settings_.hh.delay_ratio << std::endl;
  ss << "     hh_delay_wetmix:" << settings_.hh.delay_wetmix
     << " hh_delay_sync_tempo:" << settings_.hh.delay_sync_tempo
     << " hh_delay_sync_len:" << settings_.hh.delay_sync_len << std::endl;
  ss << COOL_COLOR_YELLOW_MELLOW "     oh(4): oh_vol:" << settings_.oh.vol
     << " oh_pan:" << settings_.oh.pan << " oh_attack:" << settings_.oh.attack
     << " oh_decay:" << settings_.oh.decay << std::endl;
  ss << "     oh_sqamp:" << settings_.oh.sqamp
     << " oh_midf:" << settings_.oh.midf << " oh_hif:" << settings_.oh.hif
     << " oh_hif_q:" << settings_.oh.hif_q
     << " oh_dist:" << settings_.oh.distortion_threshold << std::endl;
  ss << "     oh_delay_mode:" << settings_.oh.delay_mode
     << " oh_delay_ms:" << settings_.oh.delay_ms
     << " oh_delay_feedback_pct:" << settings_.oh.delay_feedback_pct
     << " oh_delay_ratio:" << settings_.oh.delay_ratio << std::endl;
  ss << "     oh_delay_wetmix:" << settings_.oh.delay_wetmix
     << " oh_delay_sync_tempo:" << settings_.oh.delay_sync_tempo
     << " oh_delay_sync_len:" << settings_.oh.delay_sync_len << std::endl;
  ss << COOL_COLOR_ORANGE "     fm1(5): fm1_vol:" << settings_.fm1.vol
     << " fm1_pan:" << settings_.fm1.pan
     << " fm1_car_freq:" << settings_.fm1.carrier_freq
     << " fm1_mod_freq_rat:" << settings_.fm1.modulator_freq_ratio << std::endl;
  ss << "     fm1_car_eg_attack:" << settings_.fm1.carrier_eg_attack
     << " fm1_car_eg_decay:" << settings_.fm1.carrier_eg_decay
     << " fm1_car_eg_sustain:" << settings_.fm1.carrier_eg_sustain
     << " fm1_car_eg_release:" << settings_.fm1.carrier_eg_release << std::endl;
  ss << "     fm1_mod_eg_attack:" << settings_.fm1.modulator_eg_attack
     << " fm1_mod_eg_decay:" << settings_.fm1.modulator_eg_decay
     << " fm1_mod_eg_sustain:" << settings_.fm1.modulator_eg_sustain
     << " fm1_mod_eg_release:" << settings_.fm1.modulator_eg_release
     << std::endl;
  ss << COOL_COLOR_YELLOW_MELLOW "     fm2(6): fm2_vol:" << settings_.fm2.vol
     << " fm2_pan:" << settings_.fm2.pan
     << " fm2_car_freq:" << settings_.fm2.carrier_freq
     << " fm2_mod_freq_rat:" << settings_.fm2.modulator_freq_ratio << std::endl;
  ss << "     fm2_car_eg_attack:" << settings_.fm2.carrier_eg_attack
     << " fm2_car_eg_decay:" << settings_.fm2.carrier_eg_decay
     << " fm2_car_eg_sustain:" << settings_.fm2.carrier_eg_sustain
     << " fm2_car_eg_release:" << settings_.fm2.carrier_eg_release << std::endl;
  ss << "     fm2_mod_eg_attack:" << settings_.fm2.modulator_eg_attack
     << " fm2_mod_eg_decay:" << settings_.fm2.modulator_eg_decay
     << " fm2_mod_eg_sustain:" << settings_.fm2.modulator_eg_sustain
     << " fm2_mod_eg_release:" << settings_.fm2.modulator_eg_release
     << std::endl;
  ss << COOL_COLOR_ORANGE "     fm3(7): fm3_vol:" << settings_.fm3.vol
     << " fm3_pan:" << settings_.fm3.pan
     << " fm3_car_freq:" << settings_.fm3.carrier_freq
     << " fm3_mod_freq_rat:" << settings_.fm3.modulator_freq_ratio << std::endl;
  ss << "     fm3_car_eg_attack:" << settings_.fm3.carrier_eg_attack
     << " fm3_car_eg_decay:" << settings_.fm3.carrier_eg_decay
     << " fm3_car_eg_sustain:" << settings_.fm3.carrier_eg_sustain
     << " fm3_car_eg_release:" << settings_.fm3.carrier_eg_release << std::endl;
  ss << "     fm3_mod_eg_attack:" << settings_.fm3.modulator_eg_attack
     << " fm3_mod_eg_decay:" << settings_.fm3.modulator_eg_decay
     << " fm3_mod_eg_sustain:" << settings_.fm3.modulator_eg_sustain
     << " fm3_mod_eg_release:" << settings_.fm3.modulator_eg_release
     << std::endl;
  ss << COOL_COLOR_YELLOW_MELLOW "     lz(8): lz_vol:" << settings_.lz.vol
     << " lz_pan:" << settings_.lz.pan << " lz_freq:" << settings_.lz.freq
     << " lz_attack:" << settings_.lz.attack
     << " lz_decay:" << settings_.lz.decay
     << " lz_range:" << settings_.lz.osc_range << std::endl;

  return ss.str();
}

std::string DrumSynth::Status() {
  std::stringstream ss;
  if (!active || volume == 0)
    ss << ANSI_COLOR_RESET;
  else
    ss << COOL_COLOR_YELLOW_MELLOW;
  ss << "DrumZynth - " << COOL_COLOR_PINK2 << " - vol:" << volume;

  return ss.str();
}

void DrumSynth::Start() {
  if (active) return;  // no-op
  active = true;
}

void DrumSynth::Stop() {
  if (active) return;  // no-op
  active = false;
}

// no-op
void DrumSynth::NoteOff(midi_event ev) {}

void DrumSynth::NoteOn(midi_event ev) {
  unsigned int drum_module_num = ev.data1;

  double velocity = scaleybum(0, 127, 0, 1, ev.data2);

  switch (drum_module_num) {
    case (0):
      // Bass Drum
      bd_->NoteOn(velocity);
      break;
    case (1):
      // Snare Drum
      sd_->NoteOn(velocity);
      break;
    case (2):
      // HandClap
      cp_->NoteOn(velocity);
      break;
    case (3):
      // Closed Hi Hat
      hh_->NoteOn(velocity);
      break;
    case (4):
      // Hi Hat 2 // Open Hat
      oh_->NoteOn(velocity);
      break;
    case (5):
      // FM Drum 1
      fm1_->NoteOn(velocity);
      break;
    case (6):
      // FM Drum 2
      fm2_->NoteOn(velocity);
      break;
    case (7):
      // FM Drum 3
      fm3_->NoteOn(velocity);
      break;
    case (8):
      // Lazer
      lz_->NoteOn(velocity);
      break;
    default:
      std::cerr << "DrumSynth - num not implemented:" << drum_module_num
                << std::endl;
  }
}

namespace {
// Helper: read a JSON object from a file, return empty object on failure.
nlohmann::json ReadJsonFile(const char *path) {
  std::ifstream f(path);
  if (!f.is_open()) return nlohmann::json::object();
  try {
    nlohmann::json j;
    f >> j;
    return j;
  } catch (...) {
    return nlohmann::json::object();
  }
}
// Helper: write a JSON object to a file (2-space indent).
void WriteJsonFile(const char *path, const nlohmann::json &j) {
  std::ofstream f(path);
  f << j.dump(2) << "\n";
}
}  // namespace

void DrumSynth::Save(std::string new_preset_name) {
  if (new_preset_name.empty()) {
    std::cerr << "Play tha game, pal, need a name to save yer synth settings"
              << std::endl;
    return;
  }
  settings_.name = new_preset_name;

  auto root = ReadJsonFile(DRUM_KITS_FILENAME);
  root[new_preset_name] = settings_;
  WriteJsonFile(DRUM_KITS_FILENAME, root);

  std::cout << "DRUMSYNTH -- saved kit '" << new_preset_name << "'"
            << std::endl;
}

void DrumSynth::SavePart(std::string preset_name, std::string part) {
  if (preset_name.empty()) {
    std::cerr << "SavePart: need a preset name" << std::endl;
    return;
  }
  auto save = [&](const char *file, const nlohmann::json &val) {
    auto root = ReadJsonFile(file);
    root[preset_name] = val;
    WriteJsonFile(file, root);
    std::cout << "DRUMSYNTH -- saved " << part << " preset '" << preset_name
              << "'" << std::endl;
  };
  if (part == "bd")
    save(DRUM_BD_PRESETS_FILENAME, nlohmann::json(settings_.bd));
  else if (part == "sd")
    save(DRUM_SD_PRESETS_FILENAME, nlohmann::json(settings_.sd));
  else if (part == "hh")
    save(DRUM_HH_PRESETS_FILENAME, nlohmann::json(settings_.hh));
  else if (part == "oh")
    save(DRUM_OH_PRESETS_FILENAME, nlohmann::json(settings_.oh));
  else if (part == "cp")
    save(DRUM_CP_PRESETS_FILENAME, nlohmann::json(settings_.cp));
  else if (part == "fm1")
    save(DRUM_FM_PRESETS_FILENAME, nlohmann::json(settings_.fm1));
  else if (part == "fm2")
    save(DRUM_FM_PRESETS_FILENAME, nlohmann::json(settings_.fm2));
  else if (part == "fm3")
    save(DRUM_FM_PRESETS_FILENAME, nlohmann::json(settings_.fm3));
  else if (part == "lz")
    save(DRUM_LZ_PRESETS_FILENAME, nlohmann::json(settings_.lz));
  else
    std::cerr << "SavePart: unknown part '" << part
              << "' (use bd/sd/hh/oh/cp/fm1/fm2/fm3/lz)" << std::endl;
}

bool DrumSynth::LoadPart(std::string preset_name, std::string part) {
  auto load = [&](const char *file) -> bool {
    auto root = ReadJsonFile(file);
    if (!root.contains(preset_name)) {
      std::cerr << "LoadPart: no preset '" << preset_name << "' in " << file
                << std::endl;
      return false;
    }
    return true;
  };
  try {
    if (part == "bd") {
      if (!load(DRUM_BD_PRESETS_FILENAME)) return false;
      settings_.bd =
          ReadJsonFile(DRUM_BD_PRESETS_FILENAME)[preset_name].get<BdSettings>();
    } else if (part == "sd") {
      if (!load(DRUM_SD_PRESETS_FILENAME)) return false;
      settings_.sd =
          ReadJsonFile(DRUM_SD_PRESETS_FILENAME)[preset_name].get<SdSettings>();
    } else if (part == "hh") {
      if (!load(DRUM_HH_PRESETS_FILENAME)) return false;
      settings_.hh =
          ReadJsonFile(DRUM_HH_PRESETS_FILENAME)[preset_name].get<HhSettings>();
    } else if (part == "oh") {
      if (!load(DRUM_OH_PRESETS_FILENAME)) return false;
      settings_.oh =
          ReadJsonFile(DRUM_OH_PRESETS_FILENAME)[preset_name].get<HhSettings>();
    } else if (part == "cp") {
      if (!load(DRUM_CP_PRESETS_FILENAME)) return false;
      settings_.cp =
          ReadJsonFile(DRUM_CP_PRESETS_FILENAME)[preset_name].get<CpSettings>();
    } else if (part == "fm1") {
      if (!load(DRUM_FM_PRESETS_FILENAME)) return false;
      settings_.fm1 = ReadJsonFile(DRUM_FM_PRESETS_FILENAME)[preset_name]
                          .get<FmDrumSettings>();
    } else if (part == "fm2") {
      if (!load(DRUM_FM_PRESETS_FILENAME)) return false;
      settings_.fm2 = ReadJsonFile(DRUM_FM_PRESETS_FILENAME)[preset_name]
                          .get<FmDrumSettings>();
    } else if (part == "fm3") {
      if (!load(DRUM_FM_PRESETS_FILENAME)) return false;
      settings_.fm3 = ReadJsonFile(DRUM_FM_PRESETS_FILENAME)[preset_name]
                          .get<FmDrumSettings>();
    } else if (part == "lz") {
      if (!load(DRUM_LZ_PRESETS_FILENAME)) return false;
      settings_.lz = ReadJsonFile(DRUM_LZ_PRESETS_FILENAME)[preset_name]
                         .get<LazerSettings>();
    } else {
      std::cerr << "LoadPart: unknown part '" << part
                << "' (use bd/sd/hh/oh/cp/fm1/fm2/fm3/lz)" << std::endl;
      return false;
    }
  } catch (const std::exception &e) {
    std::cerr << "LoadPart error: " << e.what() << std::endl;
    return false;
  }
  Update();
  return true;
}

void DrumSynth::SetVolume(double v) {
  settings_.volume = v;  // Update settings so Update() doesn't overwrite
  SoundGenerator::SetVolume(v);  // Call base class
}

void DrumSynth::SetPan(double p) {
  // DrumSynth doesn't have a global pan in settings, each drum has its own
  SoundGenerator::SetPan(p);
}

void DrumSynth::Update() {
  volume = settings_.volume;
  bd_->dca_.m_amplitude_control = settings_.bd.vol;
  bd_->dca_.m_pan_control = settings_.bd.pan;
  bd_->noise_->m_amplitude = settings_.bd.noise_vol;
  bd_->noise_enabled_ = settings_.bd.noise_enabled;
  bd_->out_filter_->SetFcControl(settings_.bd.tone);
  bd_->out_filter_->SetQControlGUI(settings_.bd.q);
  bd_->noise_filter_->SetFcControl(settings_.bd.ntone);
  bd_->noise_filter_->SetQControlGUI(settings_.bd.nq);
  bd_->eg_.SetDecayTimeMsec(settings_.bd.decay);
  bd_->frequency_ = settings_.bd.frequency;
  bd_->osc1_->m_cents = settings_.bd.detune_cents;
  bd_->osc2_->m_cents = -(settings_.bd.detune_cents);
  bd_->hard_sync_ = settings_.bd.hard_sync;
  bd_->use_distortion_ = settings_.bd.use_distortion;
  bd_->distortion_.SetParam("threshold", settings_.bd.distortion_threshold);
  bd_->use_delay_ = settings_.bd.use_delay;
  bd_->delay_->SetMode(settings_.bd.delay_mode);
  bd_->delay_->SetDelayTimeMs(settings_.bd.delay_ms);
  bd_->delay_->SetFeedbackPercent(settings_.bd.delay_feedback_pct);
  bd_->delay_->SetDelayRatio(settings_.bd.delay_ratio);
  bd_->delay_->SetWetMix(settings_.bd.delay_wetmix);
  bd_->delay_->SetSync(settings_.bd.delay_sync_tempo);
  bd_->delay_->SetSyncLen(settings_.bd.delay_sync_len);
  bd_->pitch_osc_range_ = settings_.bd.pitch_env_range;
  bd_->pitch_osc_range2_ = settings_.bd.pitch_env2_range;
  bd_->pitch_eg2_.SetAttackTimeMsec(settings_.bd.pitch_env2_attack);
  bd_->pitch_eg2_.SetDecayTimeMsec(settings_.bd.pitch_env2_decay);
  bd_->pitch_eg2_.Update();
  bd_->eg_.SetAttackTimeMsec(settings_.bd.attack);
  bd_->osc1_->m_waveform = settings_.bd.osc1_waveform;
  bd_->osc2_->m_waveform = settings_.bd.osc2_waveform;
  bd_->noise_eg_.SetAttackTimeMsec(settings_.bd.noise_attack);
  bd_->chirp_enabled_ = settings_.bd.chirp_enabled;
  bd_->chirp_start_freq_ = settings_.bd.chirp_start_freq;
  bd_->chirp_end_freq_ = settings_.bd.chirp_end_freq;
  bd_->chirp_decay_ms_ = settings_.bd.chirp_decay;
  bd_->chirp_amplitude_ = settings_.bd.chirp_amp;
  bd_->chirp_eg_.SetDecayTimeMsec(settings_.bd.chirp_decay);
  bd_->chirp_eg_.Update();
  bd_->mod_enabled_ = settings_.bd.mod_enabled;
  bd_->mod_freq_ = settings_.bd.mod_freq;
  bd_->mod_index_ = settings_.bd.mod_index;
  bd_->mod_eg_.SetDecayTimeMsec(settings_.bd.mod_decay);
  bd_->mod_eg_.Update();

  sd_->dca_.m_amplitude_control = settings_.sd.vol;
  sd_->dca_.m_pan_control = settings_.sd.pan;
  sd_->noise_->m_amplitude = settings_.sd.noise_vol;
  sd_->noise_eg_.SetDecayTimeMsec(settings_.sd.noise_decay);
  sd_->noise_filter_->SetFcControl(settings_.sd.tone);
  sd_->eg_.SetDecayTimeMsec(settings_.sd.decay);
  sd_->lo_osc_->m_waveform = settings_.sd.lo_osc_waveform;
  sd_->lo_osc_->m_osc_fo = settings_.sd.frequency;
  sd_->hi_osc_->m_waveform = settings_.sd.hi_osc_waveform;
  sd_->hi_osc_->m_osc_fo = sd_->lo_osc_->m_osc_fo * settings_.sd.hi_ratio;
  sd_->parallel_sat_enabled_ = settings_.sd.parallel_sat_enabled;
  sd_->parallel_sat_drive_ = settings_.sd.parallel_sat_drive;
  sd_->parallel_sat_blend_ = settings_.sd.parallel_sat_blend;
  sd_->distortion_.SetParam("threshold", settings_.sd.distortion_threshold);
  sd_->use_delay_ = settings_.sd.use_delay;
  sd_->delay_->SetMode(settings_.sd.delay_mode);
  sd_->delay_->SetDelayTimeMs(settings_.sd.delay_ms);
  sd_->delay_->SetFeedbackPercent(settings_.sd.delay_feedback_pct);
  sd_->delay_->SetDelayRatio(settings_.sd.delay_ratio);
  sd_->delay_->SetWetMix(settings_.sd.delay_wetmix);
  sd_->delay_->SetSync(settings_.sd.delay_sync_tempo);
  sd_->delay_->SetSyncLen(settings_.sd.delay_sync_len);
  sd_->eg_.SetAttackTimeMsec(settings_.sd.attack);
  sd_->noise_eg_.SetAttackTimeMsec(settings_.sd.noise_attack);
  sd_->pitch_eg_depth_ = settings_.sd.pitch_eg_depth;
  sd_->pitch_eg_.SetDecayTimeMsec(settings_.sd.pitch_eg_decay);

  hh_->dca_.m_amplitude_control = settings_.hh.vol;
  hh_->dca_.m_pan_control = settings_.hh.pan;
  hh_->eg_.SetAttackTimeMsec(settings_.hh.attack);
  hh_->eg_.SetDecayTimeMsec(settings_.hh.decay);
  hh_->SetAmplitude(settings_.hh.sqamp);
  hh_->mid_filter_->SetFcControl(settings_.hh.midf);
  hh_->high_filter_->SetFcControl(settings_.hh.hif);
  hh_->high_filter_->SetQControlGUI(settings_.hh.hif_q);
  hh_->distortion_.SetParam("threshold", settings_.hh.distortion_threshold);
  hh_->use_delay_ = settings_.hh.use_delay;
  hh_->delay_->SetMode(settings_.hh.delay_mode);
  hh_->delay_->SetDelayTimeMs(settings_.hh.delay_ms);
  hh_->delay_->SetFeedbackPercent(settings_.hh.delay_feedback_pct);
  hh_->delay_->SetDelayRatio(settings_.hh.delay_ratio);
  hh_->delay_->SetWetMix(settings_.hh.delay_wetmix);
  hh_->delay_->SetSync(settings_.hh.delay_sync_tempo);
  hh_->delay_->SetSyncLen(settings_.hh.delay_sync_len);

  oh_->dca_.m_amplitude_control = settings_.oh.vol;
  oh_->dca_.m_pan_control = settings_.oh.pan;
  oh_->eg_.SetAttackTimeMsec(settings_.oh.attack);
  oh_->eg_.SetDecayTimeMsec(settings_.oh.decay);
  oh_->SetAmplitude(settings_.oh.sqamp);
  oh_->mid_filter_->SetFcControl(settings_.oh.midf);
  oh_->high_filter_->SetFcControl(settings_.oh.hif);
  oh_->high_filter_->SetQControlGUI(settings_.oh.hif_q);
  oh_->distortion_.SetParam("threshold", settings_.oh.distortion_threshold);
  oh_->use_delay_ = settings_.oh.use_delay;
  oh_->delay_->SetMode(settings_.oh.delay_mode);
  oh_->delay_->SetDelayTimeMs(settings_.oh.delay_ms);
  oh_->delay_->SetFeedbackPercent(settings_.oh.delay_feedback_pct);
  oh_->delay_->SetDelayRatio(settings_.oh.delay_ratio);
  oh_->delay_->SetWetMix(settings_.oh.delay_wetmix);
  oh_->delay_->SetSync(settings_.oh.delay_sync_tempo);
  oh_->delay_->SetSyncLen(settings_.oh.delay_sync_len);

  cp_->dca_.m_amplitude_control = settings_.cp.vol;
  cp_->dca_.m_pan_control = settings_.cp.pan;
  cp_->voices_[0].noise->m_amplitude = settings_.cp.nvol;
  cp_->voices_[0].noise_eg.SetAttackTimeMsec(settings_.cp.nattack);
  cp_->voices_[0].noise_eg.SetDecayTimeMsec(settings_.cp.ndecay);
  cp_->voices_[0].noise_filter->SetFcControl(settings_.cp.tone);
  cp_->voices_[0].noise_filter->SetQControlGUI(settings_.cp.fq);
  cp_->voices_[0].lfo->m_waveform = settings_.cp.lfo_type;
  cp_->voices_[0].lfo->m_osc_fo = settings_.cp.lfo_rate;
  cp_->eg_.SetAttackTimeMsec(settings_.cp.eg_attack);
  cp_->eg_.SetDecayTimeMsec(settings_.cp.eg_decay);
  cp_->eg_.SetSustainLevel(settings_.cp.eg_sustain);
  cp_->eg_.SetReleaseTimeMsec(settings_.cp.eg_release);
  cp_->distortion_.SetParam("threshold", settings_.cp.distortion_threshold);
  cp_->use_delay_ = settings_.cp.use_delay;
  cp_->delay_->SetMode(settings_.cp.delay_mode);
  cp_->delay_->SetDelayTimeMs(settings_.cp.delay_ms);
  cp_->delay_->SetFeedbackPercent(settings_.cp.delay_feedback_pct);
  cp_->delay_->SetDelayRatio(settings_.cp.delay_ratio);
  cp_->delay_->SetWetMix(settings_.cp.delay_wetmix);
  cp_->delay_->SetSync(settings_.cp.delay_sync_tempo);
  cp_->delay_->SetSyncLen(settings_.cp.delay_sync_len);
  cp_->voice2_delay_ms_ = settings_.cp.v2_delay_ms;
  cp_->voice2_vol_ = settings_.cp.v2_vol;
  cp_->voice2_attack_ms_ = settings_.cp.v2_attack;
  cp_->voice2_decay_ms_ = settings_.cp.v2_decay;
  cp_->voice3_delay_ms_ = settings_.cp.v3_delay_ms;
  cp_->voice3_vol_ = settings_.cp.v3_vol;
  cp_->voice3_attack_ms_ = settings_.cp.v3_attack;
  cp_->voice3_decay_ms_ = settings_.cp.v3_decay;
  cp_->voice4_delay_ms_ = settings_.cp.v4_delay_ms;
  cp_->voice4_vol_ = settings_.cp.v4_vol;
  cp_->voice4_attack_ms_ = settings_.cp.v4_attack;
  cp_->voice4_decay_ms_ = settings_.cp.v4_decay;

  fm1_->dca_.m_amplitude_control = settings_.fm1.vol;
  fm1_->dca_.m_pan_control = settings_.fm1.pan;
  fm1_->carrier_->m_osc_fo = settings_.fm1.carrier_freq;
  fm1_->eg_.SetAttackTimeMsec(settings_.fm1.carrier_eg_attack);
  fm1_->eg_.SetDecayTimeMsec(settings_.fm1.carrier_eg_decay);
  fm1_->eg_.SetSustainLevel(settings_.fm1.carrier_eg_sustain);
  fm1_->eg_.SetReleaseTimeMsec(settings_.fm1.carrier_eg_release);
  //
  fm1_->modulator_->m_osc_fo =
      fm1_->carrier_->m_osc_fo * settings_.fm1.modulator_freq_ratio;
  fm1_->modulator_eg_.SetAttackTimeMsec(settings_.fm1.modulator_eg_attack);
  fm1_->modulator_eg_.SetDecayTimeMsec(settings_.fm1.modulator_eg_decay);
  fm1_->modulator_eg_.SetSustainLevel(settings_.fm1.modulator_eg_sustain);
  fm1_->modulator_eg_.SetReleaseTimeMsec(settings_.fm1.modulator_eg_release);

  fm2_->dca_.m_amplitude_control = settings_.fm2.vol;
  fm2_->dca_.m_pan_control = settings_.fm2.pan;
  fm2_->carrier_->m_osc_fo = settings_.fm2.carrier_freq;
  fm2_->eg_.SetAttackTimeMsec(settings_.fm2.carrier_eg_attack);
  fm2_->eg_.SetDecayTimeMsec(settings_.fm2.carrier_eg_decay);
  fm2_->eg_.SetSustainLevel(settings_.fm2.carrier_eg_sustain);
  fm2_->eg_.SetReleaseTimeMsec(settings_.fm2.carrier_eg_release);
  //
  fm2_->modulator_->m_osc_fo =
      fm2_->carrier_->m_osc_fo * settings_.fm2.modulator_freq_ratio;
  fm2_->modulator_eg_.SetAttackTimeMsec(settings_.fm2.modulator_eg_attack);
  fm2_->modulator_eg_.SetDecayTimeMsec(settings_.fm2.modulator_eg_decay);
  fm2_->modulator_eg_.SetSustainLevel(settings_.fm2.modulator_eg_sustain);
  fm2_->modulator_eg_.SetReleaseTimeMsec(settings_.fm2.modulator_eg_release);

  fm3_->dca_.m_amplitude_control = settings_.fm3.vol;
  fm3_->dca_.m_pan_control = settings_.fm3.pan;
  fm3_->carrier_->m_osc_fo = settings_.fm3.carrier_freq;
  fm3_->eg_.SetAttackTimeMsec(settings_.fm3.carrier_eg_attack);
  fm3_->eg_.SetDecayTimeMsec(settings_.fm3.carrier_eg_decay);
  fm3_->eg_.SetSustainLevel(settings_.fm3.carrier_eg_sustain);
  fm3_->eg_.SetReleaseTimeMsec(settings_.fm3.carrier_eg_release);
  //
  fm3_->modulator_->m_osc_fo =
      fm3_->carrier_->m_osc_fo * settings_.fm3.modulator_freq_ratio;
  fm3_->modulator_eg_.SetAttackTimeMsec(settings_.fm3.modulator_eg_attack);
  fm3_->modulator_eg_.SetDecayTimeMsec(settings_.fm3.modulator_eg_decay);
  fm3_->modulator_eg_.SetSustainLevel(settings_.fm3.modulator_eg_sustain);
  fm3_->modulator_eg_.SetReleaseTimeMsec(settings_.fm3.modulator_eg_release);

  lz_->dca_.m_amplitude_control = settings_.lz.vol;
  lz_->dca_.m_pan_control = settings_.lz.pan;
  lz_->osc1_->m_osc_fo = settings_.lz.freq;
  lz_->pitch_osc_range_ = settings_.lz.osc_range;
  lz_->eg_.SetAttackTimeMsec(settings_.lz.attack);
  lz_->eg_.SetDecayTimeMsec(settings_.lz.decay);
}

void DrumSynth::LoadSettings(DrumSettings settings) {
  settings_ = settings;
  Update();
}

void DrumSynth::PrintSettings(DrumSettings settingz) {
  std::cout << "AAIIGht, settings for" << settingz.name << std::endl;
}

void DrumSynth::Randomize() {}

void DrumSynth::LoadPreset(std::string name,
                           std::map<std::string, double> preset_vals) {
  // 1. Try new per-instrument kits file
  auto try_json = [&](const char *path) -> bool {
    auto root = ReadJsonFile(path);
    if (!root.contains(name)) return false;
    try {
      LoadSettings(root[name].get<DrumSettings>());
      return true;
    } catch (const std::exception &e) {
      std::cerr << "DrumSynth JSON load error (" << path << "): " << e.what()
                << std::endl;
      return false;
    }
  };
  if (try_json(DRUM_KITS_FILENAME)) return;
}

}  // namespace SBAudio
