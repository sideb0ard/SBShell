// lyria.cpp — Google Magenta Lyria RealTime audio generator
//
// Connects to the BidiGenerateMusic WebSocket API via pure POSIX + OpenSSL.
// Audio is streamed as base64-encoded 48kHz stereo int16 PCM inside JSON,
// decoded into a lock-free ring buffer, and resampled to 44100Hz in GenNext().
//
// Set LYRIA_API_KEY env var before using. Use SetParam("connect", 1) to start.
// Use SetParam("prompt", ...) isn't possible since SetParam takes double —
// instead set LYRIA_PROMPT env var, or use SetParam("connect", 1) to reconnect
// after changing the env var.

#include "lyria.h"

#include <json/json.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// POSIX
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

namespace SBAudio {

// ---------------------------------------------------------------------------
// Base64 decode (RFC 4648)
// ---------------------------------------------------------------------------
static std::vector<uint8_t> b64_decode(const std::string& in) {
  static const int8_t T[256] = {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57,
      58, 59, 60, 61, -1, -1, -1, -2, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,
      7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
      25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
      37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,
  };
  std::vector<uint8_t> out;
  out.reserve(in.size() * 3 / 4);
  int val = 0, valb = -8;
  for (unsigned char c : in) {
    int v = T[c];
    if (v == -2) break;   // padding '='
    if (v < 0) continue;  // whitespace or unknown
    val = (val << 6) | v;
    valb += 6;
    if (valb >= 0) {
      out.push_back((val >> valb) & 0xFF);
      valb -= 8;
    }
  }
  return out;
}

// ---------------------------------------------------------------------------
// Base64 encode (for WebSocket handshake key)
// ---------------------------------------------------------------------------
static std::string b64_encode(const uint8_t* data, size_t len) {
  static const char T[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve((len + 2) / 3 * 4);
  for (size_t i = 0; i < len; i += 3) {
    uint32_t v = (uint32_t)data[i] << 16;
    if (i + 1 < len) v |= (uint32_t)data[i + 1] << 8;
    if (i + 2 < len) v |= (uint32_t)data[i + 2];
    out += T[(v >> 18) & 0x3F];
    out += T[(v >> 12) & 0x3F];
    out += (i + 1 < len) ? T[(v >> 6) & 0x3F] : '=';
    out += (i + 2 < len) ? T[(v >> 0) & 0x3F] : '=';
  }
  return out;
}

// ---------------------------------------------------------------------------
// Ring buffer helpers
// ---------------------------------------------------------------------------
void LyriaGenerator::RingPush(float v) {
  uint32_t wp = ring_write_.load(std::memory_order_relaxed);
  uint32_t rp = ring_read_.load(std::memory_order_acquire);
  if ((wp - rp) >= (uint32_t)(kLyriaRingSize - 1)) return;  // full, drop
  ring_[wp & kLyriaRingMask] = v;
  ring_write_.store(wp + 1, std::memory_order_release);
}

float LyriaGenerator::RingPop() {
  uint32_t rp = ring_read_.load(std::memory_order_relaxed);
  float v = ring_[rp & kLyriaRingMask];
  ring_read_.store(rp + 1, std::memory_order_release);
  return v;
}

int LyriaGenerator::RingAvailable() const {
  uint32_t wp = ring_write_.load(std::memory_order_acquire);
  uint32_t rp = ring_read_.load(std::memory_order_relaxed);
  return (int)(wp - rp);
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
LyriaGenerator::LyriaGenerator() {
  type = LYRIA_TYPE;
  active = true;

  const char* key = getenv("LYRIA_API_KEY");
  if (key && strlen(key) > 0) {
    api_key_ = key;
  }

  const char* prompt = getenv("LYRIA_PROMPT");
  if (prompt && strlen(prompt) > 0) {
    prompt_ = prompt;
  }

  running_ = true;
  net_thread_ = std::thread(&LyriaGenerator::NetworkThread, this);

  if (!api_key_.empty()) {
    want_connect_ = true;
  }
}

LyriaGenerator::~LyriaGenerator() {
  running_ = false;
  want_connect_ = false;
  // Unblock any in-progress SSL_read by shutting down the socket.
  // Using shutdown (not close) so Disconnect() can close it cleanly after join.
  if (ssl_fd_ >= 0) ::shutdown(ssl_fd_, SHUT_RDWR);
  if (net_thread_.joinable()) net_thread_.join();
  Disconnect();
}

// ---------------------------------------------------------------------------
// GenNext — audio thread, real-time safe
// ---------------------------------------------------------------------------
StereoVal LyriaGenerator::GenNext(mixer_timing_info tinfo) {
  if (!active) return {0.0, 0.0};

  // Track mixer BPM + key changes — network thread forwards to Lyria API.
  if (tinfo.bpm > 0 && std::abs(tinfo.bpm - last_sent_bpm_) > 0.5f) {
    last_sent_bpm_ = tinfo.bpm;
    pending_bpm_.store(tinfo.bpm, std::memory_order_relaxed);
  }
  int cur_key = (int)tinfo.key;
  if (cur_key != last_sent_key_) {
    last_sent_key_ = cur_key;
    pending_key_.store(cur_key, std::memory_order_relaxed);
  }

  // Advance resampler phase: for each 44100Hz output frame, consume
  // 48000/44100 ≈ 1.0884 frames worth of 48kHz source.
  static constexpr double kRatio = 48000.0 / 44100.0;
  resample_frac_ += kRatio;

  // Each time the phase crosses 1.0, advance to the next 48kHz source frame.
  while (resample_frac_ >= 1.0) {
    resample_frac_ -= 1.0;
    src_left_[0] = src_left_[1];
    src_right_[0] = src_right_[1];
    if (RingAvailable() >= 2) {
      src_left_[1] = RingPop();
      src_right_[1] = RingPop();
    } else {
      // Buffer underrun — hold last sample
      src_left_[1] = src_left_[0];
      src_right_[1] = src_right_[0];
    }
  }

  float left = (float)(src_left_[0] * (1.0 - resample_frac_) +
                       src_left_[1] * resample_frac_);
  float right = (float)(src_right_[0] * (1.0 - resample_frac_) +
                        src_right_[1] * resample_frac_);

  StereoVal val{left * volume, right * volume};
  return Effector(val);
}

// ---------------------------------------------------------------------------
// Info / Status
// ---------------------------------------------------------------------------
std::string LyriaGenerator::Info() {
  std::ostringstream ss;
  ss << "[Lyria] " << (connected_ ? "connected" : "disconnected")
     << "  buf=" << RingAvailable() / 2 << " frames"
     << "  vol=" << (int)(volume * 100) << "\n"
     << "  prompt=\"" << prompt_ << "\"\n"
     << "  style:0-9  density:" << cfg_density_
     << "  brightness:" << cfg_brightness_ << "\n"
     << "  temperature:" << cfg_temperature_ << "  guidance:" << cfg_guidance_
     << "\n"
     << "  scale:" << cfg_scale_ << "\n"
     << "  tempo:<bpm>  pause:1  connect:0/1";
  return ss.str();
}

std::string LyriaGenerator::Status() {
  return Info();
}

// ---------------------------------------------------------------------------
// Start / Stop
// ---------------------------------------------------------------------------
void LyriaGenerator::Start() {
  active = true;
  if (!api_key_.empty() && !connected_) want_connect_ = true;
}

void LyriaGenerator::Stop() {
  active = false;
}

// ---------------------------------------------------------------------------
// SetParam
// ---------------------------------------------------------------------------
void LyriaGenerator::SetStringParam(std::string name, std::string val) {
  if (name == "prompt") {
    prompt_ = val;
    if (connected_) {
      std::string msg =
          "{\"client_content\":{\"weighted_prompts\":[{\"text\":\"" + prompt_ +
          "\",\"weight\":1.0}]}}";
      SendTextFrame(msg);
      std::cout << "[Lyria] Prompt: " << prompt_ << "\n";
    }
  } else if (name == "scale") {
    cfg_scale_ = val;
    if (connected_) {
      std::string msg =
          "{\"music_generation_config\":{\"scale\":\"" + val + "\"}}";
      SendTextFrame(msg);
    }
  }
}

void LyriaGenerator::SetParam(std::string name, double val) {
  if (name == "connect") {
    if (val >= 0.5) {
      const char* key = getenv("LYRIA_API_KEY");
      if (key && strlen(key) > 0) api_key_ = key;
      const char* prompt = getenv("LYRIA_PROMPT");
      if (prompt && strlen(prompt) > 0) prompt_ = prompt;
      want_connect_ = true;
    } else {
      want_connect_ = false;
      // Interrupt the network thread's blocking SSL_read — it will call
      // Disconnect() itself. Never touch SSL objects from this thread.
      if (ssl_fd_ >= 0) ::shutdown(ssl_fd_, SHUT_RDWR);
    }
  } else if (name == "vol" || name == "volume") {
    SetVolume(val / 100.0);
  } else if (name == "tempo" || name == "density" || name == "brightness" ||
             name == "temperature" || name == "guidance") {
    // ("tempo" used instead of "bpm" since bpm is a reserved SBShell keyword)
    std::string api_name = (name == "tempo") ? "bpm" : name;
    if (name == "density") cfg_density_ = (float)val;
    if (name == "brightness") cfg_brightness_ = (float)val;
    if (name == "temperature") cfg_temperature_ = (float)val;
    if (name == "guidance") cfg_guidance_ = (float)val;
    if (connected_) {
      std::ostringstream ss;
      ss << "{\"music_generation_config\":{\"" << api_name << "\":" << val
         << "}}";
      SendTextFrame(ss.str());
    }
  } else if (name == "pause") {
    if (connected_) SendTextFrame("{\"playback_control\":\"PAUSE\"}");
  } else if (name == "style") {
    // Numbered preset prompts — sends a new weighted_prompt to steer generation
    static const char* kStyles[] = {
        "ambient electronic music",  // 0
        "upbeat dance music",        // 1
        "dark atmospheric music",    // 2
        "jazz fusion",               // 3
        "classical orchestral",      // 4
        "lofi hip hop beats",        // 5
        "heavy metal",               // 6
        "acoustic folk",             // 7
        "cinematic film score",      // 8
        "tropical house",            // 9
    };
    int idx = (int)val;
    if (idx >= 0 && idx < 10 && connected_) {
      prompt_ = kStyles[idx];
      std::string msg =
          "{\"client_content\":{\"weighted_prompts\":[{\"text\":\"" + prompt_ +
          "\",\"weight\":1.0}]}}";
      SendTextFrame(msg);
      std::cout << "[Lyria] Style " << idx << ": " << prompt_ << "\n";
    }
  }
}

// ---------------------------------------------------------------------------
// TLS WebSocket helpers (POSIX + OpenSSL)
// ---------------------------------------------------------------------------

int LyriaGenerator::SslReadExact(void* buf, int len) {
#ifdef HAVE_OPENSSL
  SSL* ssl = static_cast<SSL*>(ssl_);
  int total = 0;
  while (total < len) {
    int n = SSL_read(ssl, static_cast<char*>(buf) + total, len - total);
    if (n <= 0) return total;
    total += n;
  }
  return total;
#else
  (void)buf;
  (void)len;
  return 0;
#endif
}

void LyriaGenerator::Connect() {
#ifndef HAVE_OPENSSL
  std::cerr << "[Lyria] OpenSSL not available — cannot connect.\n";
  return;
#else
  if (api_key_.empty()) {
    std::cerr << "[Lyria] No API key — set LYRIA_API_KEY env var.\n";
    return;
  }

  const char* host = "generativelanguage.googleapis.com";
  const char* port = "443";

  // --- Resolve and connect TCP ---
  struct addrinfo hints {
  }, *res = nullptr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host, port, &hints, &res) != 0) {
    std::cerr << "[Lyria] DNS resolution failed for " << host << "\n";
    return;
  }

  int fd = -1;
  for (struct addrinfo* rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd < 0) continue;
    if (::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
    ::close(fd);
    fd = -1;
  }
  freeaddrinfo(res);

  if (fd < 0) {
    std::cerr << "[Lyria] TCP connect failed.\n";
    return;
  }

  // --- TLS ---
  SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
  SSL_CTX_set_default_verify_paths(ctx);

  SSL* ssl = SSL_new(ctx);
  SSL_set_fd(ssl, fd);
  SSL_set_tlsext_host_name(ssl, host);  // SNI

  if (SSL_connect(ssl) != 1) {
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ::close(fd);
    std::cerr << "[Lyria] TLS handshake failed.\n";
    return;
  }

  ssl_fd_ = fd;
  ssl_ = ssl;
  ssl_ctx_ = ctx;

  // --- WebSocket handshake ---
  uint8_t ws_key_bytes[16];
  for (int i = 0; i < 16; i++) ws_key_bytes[i] = (uint8_t)(rand() & 0xFF);
  std::string ws_key = b64_encode(ws_key_bytes, 16);

  const std::string path =
      "/ws/google.ai.generativelanguage.v1alpha"
      ".GenerativeService.BidiGenerateMusic?key=" +
      api_key_;

  std::string request = "GET " + path +
                        " HTTP/1.1\r\n"
                        "Host: " +
                        std::string(host) +
                        "\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Key: " +
                        ws_key +
                        "\r\n"
                        "Sec-WebSocket-Version: 13\r\n"
                        "\r\n";

  SSL_write(ssl, request.c_str(), (int)request.size());

  // Read response headers
  std::string response;
  char ch;
  while (true) {
    if (SSL_read(ssl, &ch, 1) != 1) break;
    response += ch;
    if (response.size() >= 4 &&
        response.substr(response.size() - 4) == "\r\n\r\n")
      break;
    if (response.size() > 8192) break;  // safety valve
  }

  if (response.find("101") == std::string::npos) {
    std::cerr << "[Lyria] WebSocket upgrade rejected:\n" << response << "\n";
    Disconnect();
    return;
  }

  connected_ = true;
  std::cout << "[Lyria] Connected.\n";
  SendSetup();
#endif
}

void LyriaGenerator::Disconnect() {
#ifdef HAVE_OPENSSL
  connected_ = false;
  if (ssl_) {
    SSL_shutdown(static_cast<SSL*>(ssl_));
    SSL_free(static_cast<SSL*>(ssl_));
    ssl_ = nullptr;
  }
  if (ssl_ctx_) {
    SSL_CTX_free(static_cast<SSL_CTX*>(ssl_ctx_));
    ssl_ctx_ = nullptr;
  }
#endif
  if (ssl_fd_ >= 0) {
    ::close(ssl_fd_);
    ssl_fd_ = -1;
  }
}

// Send a masked text WebSocket frame (client → server must be masked).
bool LyriaGenerator::SendTextFrame(const std::string& json) {
#ifndef HAVE_OPENSSL
  (void)json;
  return false;
#else
  SSL* ssl = static_cast<SSL*>(ssl_);
  size_t payload_len = json.size();

  std::vector<uint8_t> frame;
  frame.push_back(0x81);  // FIN + text opcode

  // Masking key
  uint8_t mask[4];
  for (int i = 0; i < 4; i++) mask[i] = (uint8_t)(rand() & 0xFF);

  if (payload_len < 126) {
    frame.push_back(0x80 | (uint8_t)payload_len);
  } else if (payload_len < 65536) {
    frame.push_back(0x80 | 126);
    frame.push_back((uint8_t)(payload_len >> 8));
    frame.push_back((uint8_t)(payload_len & 0xFF));
  } else {
    frame.push_back(0x80 | 127);
    for (int i = 7; i >= 0; i--)
      frame.push_back((uint8_t)((payload_len >> (i * 8)) & 0xFF));
  }

  frame.insert(frame.end(), mask, mask + 4);

  for (size_t i = 0; i < payload_len; i++)
    frame.push_back((uint8_t)(json[i]) ^ mask[i % 4]);

  int written = SSL_write(ssl, frame.data(), (int)frame.size());
  return written == (int)frame.size();
#endif
}

// Receive a single (possibly reassembled from continuations) WebSocket frame.
// Returns false on connection error or close frame.
bool LyriaGenerator::RecvFrame(std::string& out) {
#ifndef HAVE_OPENSSL
  (void)out;
  return false;
#else
  out.clear();
  bool final = false;

  while (!final) {
    uint8_t header[2];
    if (SslReadExact(header, 2) != 2) return false;

    final = (header[0] & 0x80) != 0;
    uint8_t opcode = header[0] & 0x0F;
    bool masked = (header[1] & 0x80) != 0;
    uint64_t payload_len = header[1] & 0x7F;

    if (payload_len == 126) {
      uint8_t ext[2];
      if (SslReadExact(ext, 2) != 2) return false;
      payload_len = ((uint64_t)ext[0] << 8) | ext[1];
    } else if (payload_len == 127) {
      uint8_t ext[8];
      if (SslReadExact(ext, 8) != 8) return false;
      payload_len = 0;
      for (int i = 0; i < 8; i++) payload_len = (payload_len << 8) | ext[i];
    }

    uint8_t mask_key[4] = {};
    if (masked && SslReadExact(mask_key, 4) != 4) return false;

    // Close frame — read status + reason and log it
    if (opcode == 0x08) {
      if (payload_len >= 2) {
        uint8_t status[2];
        SslReadExact(status, 2);
        int code = ((int)status[0] << 8) | status[1];
        std::string reason(payload_len - 2, '\0');
        if (payload_len > 2) SslReadExact(&reason[0], (int)(payload_len - 2));
        std::cerr << "[Lyria] Server closed: code=" << code
                  << " reason=" << reason << "\n";
      } else {
        std::cerr << "[Lyria] Server closed (no reason given).\n";
      }
      return false;
    }

    // Ping — respond with pong and loop
    if (opcode == 0x09) {
      std::vector<uint8_t> ping_payload(payload_len);
      if (payload_len > 0) SslReadExact(ping_payload.data(), (int)payload_len);
      // Pong: FIN + opcode 0xA, masked, no payload
      uint8_t pong[6] = {0x8A, 0x80, 0, 0, 0, 0};
      SSL_write(static_cast<SSL*>(ssl_), pong, 6);
      final = false;  // keep reading
      continue;
    }

    // Text or continuation frame — accumulate
    if (payload_len > 0) {
      size_t offset = out.size();
      out.resize(offset + payload_len);
      if (SslReadExact(&out[offset], (int)payload_len) != (int)payload_len)
        return false;
      if (masked) {
        for (size_t i = 0; i < payload_len; i++)
          out[offset + i] ^= mask_key[i % 4];
      }
    }
  }

  return true;
#endif
}

void LyriaGenerator::SendSetup() {
  // Send BidiGenerateMusicSetup
  std::string setup = R"({"setup":{"model":"models/lyria-realtime-exp"}})";
  SendTextFrame(setup);
}

void LyriaGenerator::HandleAudioData(const std::string& b64) {
  std::vector<uint8_t> pcm = b64_decode(b64);

  // PCM: 48kHz, stereo, signed 16-bit little-endian, interleaved L/R
  for (size_t i = 0; i + 3 < pcm.size(); i += 4) {
    int16_t left_raw =
        (int16_t)((uint16_t)pcm[i] | ((uint16_t)pcm[i + 1] << 8));
    int16_t right_raw =
        (int16_t)((uint16_t)pcm[i + 2] | ((uint16_t)pcm[i + 3] << 8));
    RingPush(left_raw / 32768.0f);
    RingPush(right_raw / 32768.0f);
  }
}

void LyriaGenerator::HandleFrame(const std::string& frame) {
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errs;
  std::istringstream ss(frame);
  if (!Json::parseFromStream(builder, ss, &root, &errs)) {
    // Log unparseable frames (likely server error text)
    std::cerr << "[Lyria] Unparseable frame: " << frame.substr(0, 512) << "\n";
    return;
  }

  // Log any error responses from the server
  if (root.isMember("error")) {
    std::cerr << "[Lyria] API error: " << root["error"].toStyledString()
              << "\n";
    return;
  }

  if (root.isMember("setupComplete")) {
    std::cout << "[Lyria] Setup complete — audio stream starting.\n";
    // Send initial prompt + start playback
    std::string content =
        "{\"client_content\":{\"weighted_prompts\":[{\"text\":\"" + prompt_ +
        "\",\"weight\":1.0}]}}";
    SendTextFrame(content);
    SendTextFrame("{\"playback_control\":\"PLAY\"}");
    return;
  }

  if (root.isMember("serverContent")) {
    const auto& sc = root["serverContent"];

    if (sc.isMember("audioChunks")) {
      for (const auto& chunk : sc["audioChunks"]) {
        const std::string& data = chunk["data"].asString();
        if (!data.empty()) HandleAudioData(data);
      }
    }
    return;
  }

  // Log any unrecognised top-level keys so we can see the real API format
  static bool logged_unknown = false;
  if (!logged_unknown) {
    logged_unknown = true;
    std::cerr << "[Lyria] Unrecognised frame keys:";
    for (const auto& key : root.getMemberNames()) std::cerr << " " << key;
    std::cerr << "\n";
    // Print a short excerpt of the raw frame
    std::cerr << "[Lyria] Frame excerpt: " << frame.substr(0, 300) << "\n";
  }
}

// ---------------------------------------------------------------------------
// Network thread — runs for the lifetime of the generator
// ---------------------------------------------------------------------------
void LyriaGenerator::NetworkThread() {
  while (running_) {
    if (!want_connect_ || api_key_.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    if (!connected_) {
      Connect();
      if (!connected_) {
        for (int i = 0; i < 50 && running_; i++)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
    }

    std::string frame;
    if (!RecvFrame(frame)) {
      if (!running_) break;
      std::cerr << "[Lyria] Connection lost — reconnecting in 3s.\n";
      Disconnect();
      for (int i = 0; i < 30 && running_; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    HandleFrame(frame);

    // Forward pending BPM / key changes detected by GenNext
    float bpm = pending_bpm_.exchange(0, std::memory_order_relaxed);
    if (bpm > 0 && connected_) {
      std::ostringstream ss;
      ss << "{\"music_generation_config\":{\"bpm\":" << (int)bpm << "}}";
      SendTextFrame(ss.str());
    }

    int key = pending_key_.exchange(-1, std::memory_order_relaxed);
    if (key >= 0 && connected_) {
      static const char* kScales[12] = {
          "C_MAJOR_A_MINOR",            // C  0
          "D_FLAT_MAJOR_B_FLAT_MINOR",  // C# 1
          "D_MAJOR_B_MINOR",            // D  2
          "E_FLAT_MAJOR_C_MINOR",       // D# 3
          "E_MAJOR_D_FLAT_MINOR",       // E  4
          "F_MAJOR_D_MINOR",            // F  5
          "G_FLAT_MAJOR_E_FLAT_MINOR",  // F# 6
          "G_MAJOR_E_MINOR",            // G  7
          "A_FLAT_MAJOR_F_MINOR",       // G# 8
          "A_MAJOR_G_FLAT_MINOR",       // A  9
          "B_FLAT_MAJOR_G_MINOR",       // A# 10
          "B_MAJOR_A_FLAT_MINOR",       // B  11
      };
      if (key < 12) {
        cfg_scale_ = kScales[key];
        std::string msg =
            std::string("{\"music_generation_config\":{\"scale\":\"") +
            cfg_scale_ + "\"}}";
        SendTextFrame(msg);
      }
    }
  }

  Disconnect();
}

}  // namespace SBAudio
