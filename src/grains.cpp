#include <audioutils.h>
#include <grains.h>

#include <cmath>
#include <iostream>

namespace {
void check_idx(int *index, int buffer_len) {
  if (buffer_len <= 0) {
    *index = 0;
    return;
  }
  while (*index < 0) *index += buffer_len;
  while (*index >= buffer_len) *index -= buffer_len;
}
}  // namespace

namespace SBAudio {

void SoundGrainSample::Initialize(SoundGrainParams params) {
  if (params.grain_type != type) {
    std::cerr << "Putting DIESEL IN AN UNLEADED!!\n";
    return;
  }

  grain_frame_counter = 0;
  incr = 1;

  audio_buffer = params.audio_buffer;
  audiobuffer_num_channels = params.num_channels;
  degrade_by = params.degrade_by;
  reverse_mode = params.reverse_mode;
  pitch_ratio = params.pitch_ratio;

  grain_len_frames = params.dur_frames;
  envelope_shape = params.envelope_shape;
  overlap_fraction = params.overlap_fraction;
  channel_mask = params.channel_mask;
  audiobuffer_cur_pos = params.starting_idx;
  if (reverse_mode) {
    audiobuffer_cur_pos =
        params.starting_idx + (grain_len_frames * audiobuffer_num_channels) - 1;
    incr = -1.0;
  }

  active = true;
}

void SoundGrainSample::SetReadIdx(int idx) {
  audiobuffer_cur_pos = idx;
}

StereoVal SoundGrainSample::Generate() {
  StereoVal out = {0., 0.};
  if (!active) return out;

  if (degrade_by > 0) {
    if (rand() % 100 < degrade_by) return out;
  }

  int buf_size = audio_buffer->size();
  int idx0 = (int)audiobuffer_cur_pos;
  check_idx(&idx0, audio_buffer->size());
  int idx0_right = idx0 + 1;
  check_idx(&idx0_right, buf_size);
  int idx1 = idx0 + audiobuffer_num_channels;
  check_idx(&idx1, audio_buffer->size());
  int idx1_right = idx1 + 1;
  check_idx(&idx1_right, buf_size);
  float frac = (float)(audiobuffer_cur_pos - std::floor(audiobuffer_cur_pos));

  out.left =
      (*audio_buffer)[idx0] * (1.0f - frac) + (*audio_buffer)[idx1] * frac;
  if (audiobuffer_num_channels == 1) {
    out.right = out.left;
  } else {
    out.right = (*audio_buffer)[idx0_right] * (1.0f - frac) +
                (*audio_buffer)[idx1_right] * frac;
  }

  audiobuffer_cur_pos += (incr * pitch_ratio * audiobuffer_num_channels);
  grain_frame_counter++;

  if (grain_frame_counter > grain_len_frames) {
    active = false;
  }

  // Apply per-grain amplitude envelope
  if (grain_len_frames > 0) {
    double t = static_cast<double>(grain_frame_counter) / grain_len_frames;
    double env = 1.0;
    if (envelope_shape == 1) {
      // Hann window: smooth bell, zero at both ends
      env = 0.5 * (1.0 - std::cos(2.0 * M_PI * t));
    } else {
      // Tukey window: flat top with cosine-tapered edges
      // Sums to 1.0 across overlapping grains when spacing=(1-f)*duration
      double f = (overlap_fraction > 0.0) ? overlap_fraction : 0.0;
      if (f > 0.0 && t < f) {
        env = 0.5 * (1.0 - std::cos(M_PI * t / f));
      } else if (f > 0.0 && t > 1.0 - f) {
        env = 0.5 * (1.0 + std::cos(M_PI * (t - (1.0 - f)) / f));
      }
      // else env = 1.0 (flat top)
    }
    out.left *= env;
    out.right *= env;
  }

  if (channel_mask == 1)
    out.right = 0.0;
  else if (channel_mask == 2)
    out.left = 0.0;

  return out;
}

}  // namespace SBAudio
