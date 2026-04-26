#include <math.h>
#include <sbsynth.h>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "defjams.h"
#include "utils.h"

namespace SBAudio {

SBSynth::SBSynth() {
  type = SBSYNTH_TYPE;
  active = true;

  filter_.SetFcControl(filter_cutoff_);
  filter_.SetQControlGUI(filter_q_);
  filter_.Update();
}

void SBSynth::AddBuffer(std::unique_ptr<FileBuffer> fb) {
  file_buffers_.push_back(std::move(fb));
}

// ─────────────────────────────────────────────────────────────────────────────
// Voice management

int SBSynth::FindFreeVoice(int note) const {
  // 1. Retrigger same note if it's already playing
  for (int i = 0; i < kSBSynthNumVoices; i++)
    if (voices_[i].active && voices_[i].note == note) return i;
  // 2. Find a truly idle voice
  for (int i = 0; i < kSBSynthNumVoices; i++)
    if (!voices_[i].active) return i;
  // 3. Steal a voice that's in release
  for (int i = 0; i < kSBSynthNumVoices; i++)
    if (voices_[i].eg.m_state == RELEASE) return i;
  // 4. Last resort: steal voice 0
  return 0;
}

double SBSynth::CalcIncrement(int note) const {
  if (file_buffers_.empty()) return 1.0;

  // Always use the first buffer as the reference for pitch calculation.
  FileBuffer *ref = file_buffers_[0].get();
  int num_ch = ref->num_channels_;
  size_t total_samples = ref->GetAudioBuffer()->size();
  size_t samples_per_channel = total_samples / (size_t)num_ch;

  if (mode_ == Mode::WAVETABLE) {
    // Cycle through the buffer at the frequency for this note.
    // increment = (samples_in_one_cycle * target_freq / sample_rate) * num_ch
    double freq = 440.0 * pow(2.0, (note - 69) / 12.0);
    return ((double)samples_per_channel * freq / SAMPLE_RATE) * num_ch;
  } else {
    // SAMPLE: pitch-shift relative to root_note_ (1x speed at root).
    double pitch_ratio = pow(2.0, (note - root_note_) / 12.0);
    return pitch_ratio * num_ch;
  }
}

void SBSynth::InitVoice(SBSynthVoice &v, int note, int velocity) {
  v.note = note;
  v.velocity = velocity;
  // Scatter: randomise start position within the buffer on each note-on
  if (scatter_ > 0.0 && !file_buffers_.empty()) {
    auto *buf = file_buffers_[0]->GetAudioBuffer();
    if (buf && !buf->empty()) {
      double r = (double)rand() / (double)RAND_MAX;
      v.read_pos = r * scatter_ * (double)buf->size();
    } else {
      v.read_pos = 0.0;
    }
  } else {
    v.read_pos = 0.0;
  }
  v.increment = CalcIncrement(note);
  v.active = true;

  v.eg.SetEgMode(ANALOG);
  v.eg.SetAttackTimeMsec(attack_ms_);
  v.eg.SetDecayTimeMsec(decay_ms_);
  v.eg.SetSustainLevel(sustain_lvl_);
  v.eg.SetReleaseTimeMsec(release_ms_);
  v.eg.m_output_eg = true;
  v.eg.m_reset_to_zero = true;
  v.eg.StartEg();
}

void SBSynth::NoteOn(midi_event ev) {
  if (file_buffers_.empty()) return;
  int idx = FindFreeVoice(ev.data1);
  InitVoice(voices_[idx], ev.data1, ev.data2 > 0 ? ev.data2 : 100);
}

void SBSynth::NoteOff(midi_event ev) {
  for (auto &v : voices_)
    if (v.active && v.note == ev.data1) {
      v.eg.NoteOff();
      break;
    }
}

void SBSynth::AllNotesOff() {
  for (auto &v : voices_)
    if (v.active) v.eg.NoteOff();
}

// ─────────────────────────────────────────────────────────────────────────────
// Audio generation

// Read one stereo sample pair from a buffer at a fractional position.
// Handles mono/stereo and wraps the position within the buffer.
static void ReadSample(const std::vector<double> *buf, int num_ch,
                       double read_pos, double &left_out, double &right_out) {
  int total = (int)buf->size();
  int idx1 = (int)read_pos;
  if (idx1 < 0 || idx1 >= total) {
    left_out = right_out = 0.0;
    return;
  }
  double frac = read_pos - idx1;
  int idx2 = idx1 + num_ch;
  if (idx2 >= total) idx2 = idx1;
  if (num_ch == 1) {
    left_out = (*buf)[idx1] * (1.0 - frac) + (*buf)[idx2] * frac;
    right_out = left_out;
  } else {
    left_out = (*buf)[idx1] * (1.0 - frac) + (*buf)[idx2] * frac;
    right_out = (*buf)[idx1 + 1] * (1.0 - frac) + (*buf)[idx2 + 1] * frac;
  }
}

StereoVal SBSynth::GenNext(mixer_timing_info tinfo) {
  (void)tinfo;

  StereoVal out{0.0, 0.0};
  if (!active || file_buffers_.empty()) return out;

  // Reference buffer (first one) drives loop length / voice lifecycle.
  FileBuffer *ref_fb = file_buffers_[0].get();
  std::vector<double> *ref_buf = ref_fb->GetAudioBuffer();
  if (!ref_buf || ref_buf->empty()) return out;

  int num_ch = ref_fb->num_channels_;
  int ref_total = (int)ref_buf->size();

  // Compute morph indices and blend factor once per audio frame.
  int n_bufs = (int)file_buffers_.size();
  double morph_scaled = morph_ * (n_bufs - 1);
  int buf_a_idx = (int)morph_scaled;
  if (buf_a_idx >= n_bufs - 1) buf_a_idx = n_bufs - 1;
  int buf_b_idx = buf_a_idx + 1 < n_bufs ? buf_a_idx + 1 : buf_a_idx;
  double t = morph_scaled - buf_a_idx;  // 0..1 blend between a and b

  std::vector<double> *buf_a = file_buffers_[buf_a_idx]->GetAudioBuffer();
  std::vector<double> *buf_b = file_buffers_[buf_b_idx]->GetAudioBuffer();

  for (auto &v : voices_) {
    if (!v.active) continue;

    double amp_env = v.eg.DoEnvelope(nullptr);

    if (amp_env == 0.0 && v.eg.m_state == OFFF) {
      v.active = false;
      continue;
    }

    int idx1 = (int)v.read_pos;
    if (idx1 < 0 || idx1 >= ref_total) {
      v.active = false;
      continue;
    }

    // Phase distortion: warp normalized phase (0..1) before sampling.
    // read_pos advances linearly (correct pitch); sample_pos is the distorted
    // address.
    double norm_phase = v.read_pos / (double)ref_total;
    double dist_norm = norm_phase;
    if (pd_amt_ > 0.0) {
      if (pd_type_ == 0) {
        // Sine warp: periodic, adds harmonics smoothly
        dist_norm = norm_phase + pd_amt_ * 0.5 * sin(norm_phase * TWO_PI);
        dist_norm = dist_norm - floor(dist_norm);  // wrap to 0..1
      } else {
        // Power curve: rushes through start, dwells on end
        dist_norm = pow(norm_phase, 1.0 + pd_amt_ * 4.0);
      }
    }
    double sample_pos = dist_norm * (double)ref_total;

    // Read from both buffers at the distorted position.
    // For buf_b, scale proportionally if it has a different length.
    double la, ra, lb, rb;
    ReadSample(buf_a, num_ch, sample_pos, la, ra);

    if (buf_b != buf_a) {
      int total_b = (int)buf_b->size();
      double pos_b = (total_b > 0)
                         ? sample_pos * (double)total_b / (double)ref_total
                         : sample_pos;
      ReadSample(buf_b, num_ch, pos_b, lb, rb);
    } else {
      lb = la;
      rb = ra;
    }

    double left_samp = la * (1.0 - t) + lb * t;
    double right_samp = ra * (1.0 - t) + rb * t;

    double vel_scale = v.velocity / 127.0;
    out.left += left_samp * amp_env * vel_scale;
    out.right += right_samp * amp_env * vel_scale;

    v.read_pos += v.increment;

    // Skip: randomly jump forward by up to 25% of the buffer
    if (skip_ > 0.0 && ((double)rand() / RAND_MAX) < skip_) {
      double jump = ((double)rand() / RAND_MAX) * (double)ref_total * 0.25;
      v.read_pos += jump;
    }

    if (v.read_pos >= (double)ref_total) {
      if (loop_)
        v.read_pos = fmod(v.read_pos, (double)ref_total);
      else
        v.active = false;
    }
  }

  // Global Moog filter on summed output
  filter_.Update();
  out.left = filter_.DoFilter(out.left);
  out.right = filter_.DoFilter(out.right);

  double pan_left = 0.707, pan_right = 0.707;
  calculate_pan_values(pan, &pan_left, &pan_right);
  out.left *= volume * pan_left;
  out.right *= volume * pan_right;

  return Effector(out);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parameters

void SBSynth::SetParam(std::string name, double val) {
  if (name == "attack") {
    attack_ms_ = val;
  } else if (name == "decay") {
    decay_ms_ = val;
  } else if (name == "sustain") {
    sustain_lvl_ = val;
  } else if (name == "release") {
    release_ms_ = val;
  } else if (name == "cutoff") {
    filter_cutoff_ = val;
    filter_.SetFcControl(filter_cutoff_);
  } else if (name == "q" || name == "resonance") {
    filter_q_ = std::max(1.0, std::min(10.0, val));
    filter_.SetQControlGUI(filter_q_);
  } else if (name == "root") {
    root_note_ = (int)val;
  } else if (name == "loop") {
    loop_ = (val != 0.0);
  } else if (name == "mode") {
    if ((int)val == 0) {
      mode_ = Mode::WAVETABLE;
      loop_ = true;
    } else {
      mode_ = Mode::SAMPLE;
      loop_ = false;
    }
  } else if (name == "morph") {
    morph_ = std::max(0.0, std::min(1.0, val));
  } else if (name == "pd") {
    pd_amt_ = std::max(0.0, std::min(1.0, val));
  } else if (name == "pd_type") {
    pd_type_ = (int)val == 0 ? 0 : 1;
  } else if (name == "scatter") {
    scatter_ = std::max(0.0, std::min(1.0, val));
  } else if (name == "skip") {
    skip_ = std::max(0.0, std::min(1.0, val));
  }
}

void SBSynth::Start() {
  active = true;
}
void SBSynth::Stop() {
  AllNotesOff();
}

// ─────────────────────────────────────────────────────────────────────────────
// Info / Status

std::string SBSynth::Status() {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  if (!active || volume == 0)
    ss << ANSI_COLOR_RESET;
  else
    ss << COOL_COLOR_BLUE_BLUEPINK2;
  ss << "SBSynth(" << (mode_ == Mode::WAVETABLE ? "wavetable" : "sample") << ")"
     << " bufs:" << file_buffers_.size() << " vol:" << volume << " pan:" << pan
     << " cut:" << (int)filter_cutoff_ << " q:" << filter_q_;
  if (mode_ == Mode::WAVETABLE && file_buffers_.size() > 1)
    ss << " morph:" << morph_;
  if (pd_amt_ > 0.0)
    ss << " pd:" << pd_amt_ << "(" << (pd_type_ == 0 ? "sine" : "pow") << ")";
  if (scatter_ > 0.0) ss << " scatter:" << scatter_;
  if (skip_ > 0.0) ss << " skip:" << skip_;
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

std::string SBSynth::Info() {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  ss << COOL_COLOR_BLUE_BLUEPINK2 << "## SBSynth ##\n" << ANSI_COLOR_WHITE;
  ss << "  mode:    "
     << (mode_ == Mode::WAVETABLE ? "0 (wavetable)" : "1 (sample)") << "\n";
  if (file_buffers_.empty()) {
    ss << "  bufs:    (none - use add_buf)\n";
  } else {
    for (int i = 0; i < (int)file_buffers_.size(); i++)
      ss << "  buf[" << i << "]:  " << file_buffers_[i]->filename_ << "\n";
  }
  ss << COOL_COLOR_BLUE_BLUEPINK2;
  ss << "  voices:  " << kSBSynthNumVoices << " max";
  int av = 0;
  for (auto &v : voices_)
    if (v.active) av++;
  ss << " (" << av << " active)\n\n";
  ss << "  attack:  " << attack_ms_ << " ms\n";
  ss << "  decay:   " << decay_ms_ << " ms\n";
  ss << "  sustain: " << sustain_lvl_ << "\n";
  ss << "  release: " << release_ms_ << " ms\n\n";
  ss << "  cutoff:  " << filter_cutoff_ << " Hz\n";
  ss << "  q:       " << filter_q_ << "\n\n";
  ss << "  root:    " << root_note_
     << "  (sample mode: MIDI note played at 1x speed)\n";
  ss << "  loop:    " << (loop_ ? "1" : "0") << "\n";
  ss << COOL_COLOR_PINK2;
  if (mode_ == Mode::WAVETABLE && file_buffers_.size() > 1)
    ss << "  morph:   " << morph_ << "  (0.0=buf[0] .. 1.0=buf["
       << file_buffers_.size() - 1 << "])\n";
  ss << "  pd:      " << pd_amt_ << "  (0.0=off .. 1.0=full)\n";
  ss << "  pd_type: " << pd_type_ << "  (0=sine warp, 1=power curve)\n";
  ss << "  scatter: " << scatter_
     << "  (0=always start at 0, 1=random start position)\n";
  ss << "  skip:    " << skip_ << "  (per-sample jump prob; try 0.001-0.02)\n";
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

}  // namespace SBAudio
