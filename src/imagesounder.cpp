#define STB_IMAGE_IMPLEMENTATION
#include "imagesounder.h"

#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <sstream>

#include "defjams.h"

namespace SBAudio {

ImageSounder::ImageSounder(std::string filepath) : filepath_(filepath) {
  type = IMAGESOUNDER_TYPE;
  active = true;

  int channels;
  unsigned char* data =
      stbi_load(filepath.c_str(), &width_, &height_, &channels, 1);
  if (!data) {
    std::cerr << "ImageSounder: failed to load " << filepath << "\n";
    return;
  }

  pixels_.resize(width_, std::vector<float>(height_));
  for (int x = 0; x < width_; x++)
    for (int y = 0; y < height_; y++)
      pixels_[x][y] = data[y * width_ + x] / 255.0f;

  stbi_image_free(data);
  phases_.resize(height_, 0.0);

  col_norms_.resize(width_);
  for (int x = 0; x < width_; x++) {
    float energy = 0;
    for (int r = 0; r < height_; r++) energy += pixels_[x][r] * pixels_[x][r];
    col_norms_[x] = std::sqrt(std::max(energy, 1e-6f));
  }
}

StereoVal ImageSounder::GenNext(mixer_timing_info tinfo) {
  if (pixels_.empty() || !active) return StereoVal{};

  double out = 0.0;

  if (fade_idx_ >= 0) {
    double t = (double)fade_idx_ / fade_samples_;
    const auto& prev_col = pixels_[prev_col_idx_];
    const auto& cur_col = pixels_[col_idx_];
    for (int r = 0; r < height_; r++) {
      double freq = lo_freq_ * std::pow(hi_freq_ / lo_freq_,
                                        (double)r / std::max(height_ - 1, 1));
      phases_[r] += TWO_PI * freq / SAMPLE_RATE;
      if (phases_[r] > TWO_PI) phases_[r] -= TWO_PI;
      double blended = prev_col[r] * (1.0 - t) + cur_col[r] * t;
      out += blended * std::sin(phases_[r]);
    }
    double norm =
        col_norms_[prev_col_idx_] * (1.0 - t) + col_norms_[col_idx_] * t;
    out = out * gain_ / std::max((double)norm, 1e-6);
    if (++fade_idx_ >= fade_samples_) fade_idx_ = -1;
  } else {
    const auto& col = pixels_[col_idx_];
    for (int r = 0; r < height_; r++) {
      double freq = lo_freq_ * std::pow(hi_freq_ / lo_freq_,
                                        (double)r / std::max(height_ - 1, 1));
      phases_[r] += TWO_PI * freq / SAMPLE_RATE;
      if (phases_[r] > TWO_PI) phases_[r] -= TWO_PI;
      out += col[r] * std::sin(phases_[r]);
    }
    out = out * gain_ / col_norms_[col_idx_];

    if (++col_sample_ >= samples_per_col_) {
      col_sample_ = 0;
      prev_col_idx_ = col_idx_;
      col_idx_ = (col_idx_ + 1) % width_;
      fade_idx_ = 0;
    }
  }

  StereoVal result{};
  result.left = out * volume;
  result.right = out * volume;
  return Effector(result);
}

void ImageSounder::SetParam(std::string name, double val) {
  if (name == "speed")
    samples_per_col_ = std::max(1, (int)val);
  else if (name == "lo")
    lo_freq_ = val;
  else if (name == "hi")
    hi_freq_ = val;
  else if (name == "gain")
    gain_ = val;
}

std::string ImageSounder::Info() {
  return "ImageSounder: " + filepath_;
}

std::string ImageSounder::Status() {
  std::stringstream ss;
  ss << "img:" << filepath_ << " " << width_ << "x" << height_
     << " col:" << col_idx_ << "/" << width_ << " speed:" << samples_per_col_
     << " lo:" << lo_freq_ << "hz hi:" << hi_freq_ << "hz";
  return ss.str();
}

}  // namespace SBAudio
