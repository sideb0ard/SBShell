#pragma once

#include <soundgenerator.h>

#include <array>
#include <atomic>
#include <string>
#include <thread>

namespace SBAudio {

// Ring buffer size: 2^19 = 524288 floats ≈ 5.9s at 48kHz stereo.
// Network thread writes float samples (decoded from int16 PCM at 48kHz).
// Audio thread reads and resamples to 44100Hz.
static constexpr int kLyriaRingBits = 19;
static constexpr int kLyriaRingSize = 1 << kLyriaRingBits;
static constexpr int kLyriaRingMask = kLyriaRingSize - 1;

class LyriaGenerator : public SoundGenerator {
 public:
  LyriaGenerator();
  ~LyriaGenerator() override;

  StereoVal GenNext(mixer_timing_info tinfo) override;
  std::string Info() override;
  std::string Status() override;
  void Start() override;
  void Stop() override;
  void SetParam(std::string name, double val) override;
  void SetStringParam(std::string name, std::string val) override;

 private:
  // --- Network thread ---
  void NetworkThread();
  void Connect();
  void Disconnect();
  bool SendTextFrame(const std::string& json);
  bool RecvFrame(std::string& out);
  void SendSetup();
  void HandleFrame(const std::string& frame);
  void HandleAudioData(const std::string& b64);
  int SslReadExact(void* buf, int len);

  // --- SPSC ring buffer (network writes, audio reads) ---
  std::array<float, kLyriaRingSize> ring_{};
  std::atomic<uint32_t> ring_write_{0};
  std::atomic<uint32_t> ring_read_{0};

  void RingPush(float v);
  float RingPop();
  int RingAvailable() const;

  // --- Resampler: 48kHz → 44100Hz linear interpolation ---
  // frac_ advances by 48000/44100 per output sample
  double resample_frac_{0.0};
  float src_left_[2]{0.0f, 0.0f};  // [current, next] source frames
  float src_right_[2]{0.0f, 0.0f};

  // --- State ---
  std::thread net_thread_;
  std::atomic<bool> running_{false};
  std::atomic<bool> connected_{false};
  std::atomic<bool> want_connect_{false};

  // BPM + key sync: GenNext writes here when mixer values change;
  // network thread reads and forwards to the API.
  std::atomic<float> pending_bpm_{0};
  float last_sent_bpm_{0};
  std::atomic<int> pending_key_{-1};  // 0-11, -1 = no change
  int last_sent_key_{-1};

  // Raw socket FD + opaque SSL pointers (avoid OpenSSL headers in .h)
  int ssl_fd_{-1};
  void* ssl_{nullptr};
  void* ssl_ctx_{nullptr};

  std::string api_key_;
  std::string prompt_{"ambient electronic music"};

  // music_generation_config state (mirrors what was last sent to the API)
  float cfg_density_{0.5f};
  float cfg_brightness_{0.5f};
  float cfg_temperature_{1.0f};
  float cfg_guidance_{3.0f};
  std::string cfg_scale_{"SCALE_UNSPECIFIED"};
};

}  // namespace SBAudio
