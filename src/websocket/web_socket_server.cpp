#include "websocket/web_socket_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

namespace {

// --- Inline SHA-1 (RFC 3174, public domain) ---

static uint32_t sha1_rol(uint32_t v, int n) {
  return (v << n) | (v >> (32 - n));
}

static void sha1_block(uint32_t h[5], const uint8_t* blk) {
  uint32_t w[80];
  for (int i = 0; i < 16; ++i)
    w[i] = (uint32_t(blk[i * 4]) << 24) | (uint32_t(blk[i * 4 + 1]) << 16) |
           (uint32_t(blk[i * 4 + 2]) << 8) | blk[i * 4 + 3];
  for (int i = 16; i < 80; ++i)
    w[i] = sha1_rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

  uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
  for (int i = 0; i < 80; ++i) {
    uint32_t f, k;
    if (i < 20) {
      f = (b & c) | (~b & d);
      k = 0x5A827999;
    } else if (i < 40) {
      f = b ^ c ^ d;
      k = 0x6ED9EBA1;
    } else if (i < 60) {
      f = (b & c) | (b & d) | (c & d);
      k = 0x8F1BBCDC;
    } else {
      f = b ^ c ^ d;
      k = 0xCA62C1D6;
    }
    uint32_t t = sha1_rol(a, 5) + f + e + k + w[i];
    e = d;
    d = c;
    c = sha1_rol(b, 30);
    b = a;
    a = t;
  }
  h[0] += a;
  h[1] += b;
  h[2] += c;
  h[3] += d;
  h[4] += e;
}

static void sha1(const void* data, size_t len, uint8_t digest[20]) {
  uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
  const uint8_t* p = static_cast<const uint8_t*>(data);
  size_t remaining = len;

  while (remaining >= 64) {
    sha1_block(h, p);
    p += 64;
    remaining -= 64;
  }

  uint8_t buf[128] = {};
  std::memcpy(buf, p, remaining);
  buf[remaining] = 0x80;
  size_t pad_len = (remaining < 56) ? 64 : 128;
  uint64_t bit_len = uint64_t(len) * 8;
  for (int i = 0; i < 8; ++i)
    buf[pad_len - 8 + i] = uint8_t(bit_len >> (56 - 8 * i));

  sha1_block(h, buf);
  if (pad_len == 128) sha1_block(h, buf + 64);

  for (int i = 0; i < 5; ++i) {
    digest[i * 4 + 0] = h[i] >> 24;
    digest[i * 4 + 1] = h[i] >> 16;
    digest[i * 4 + 2] = h[i] >> 8;
    digest[i * 4 + 3] = h[i];
  }
}

// --- Base64 encoder ---

static const char kB64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const uint8_t* data, size_t len) {
  std::string out;
  out.reserve(((len + 2) / 3) * 4);
  for (size_t i = 0; i < len; i += 3) {
    uint32_t v = uint32_t(data[i]) << 16;
    if (i + 1 < len) v |= uint32_t(data[i + 1]) << 8;
    if (i + 2 < len) v |= data[i + 2];
    out += kB64[(v >> 18) & 63];
    out += kB64[(v >> 12) & 63];
    out += (i + 1 < len) ? kB64[(v >> 6) & 63] : '=';
    out += (i + 2 < len) ? kB64[v & 63] : '=';
  }
  return out;
}

// --- WebSocket accept key ---

static std::string ws_accept_key(const std::string& key) {
  const std::string magic = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  uint8_t digest[20];
  sha1(magic.data(), magic.size(), digest);
  return base64_encode(digest, 20);
}

}  // namespace

WebsocketServer::~WebsocketServer() {
  stop();
}

void WebsocketServer::run(int port) {
  server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    std::cerr << "WebSocket: socket() failed\n";
    return;
  }

  int opt = 1;
  setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (::bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
      0) {
    std::cerr << "WebSocket: bind() failed on port " << port << "\n";
    ::close(server_fd_);
    server_fd_ = -1;
    return;
  }

  ::listen(server_fd_, 8);
  running_ = true;
  std::cout << "WebSocket server listening on port " << port << "\n";

  while (running_) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(server_fd_, &fds);
    timeval tv{1, 0};  // 1 second timeout so we can check running_
    if (::select(server_fd_ + 1, &fds, nullptr, nullptr, &tv) > 0) {
      int client_fd = ::accept(server_fd_, nullptr, nullptr);
      if (client_fd >= 0) {
        // Prevent SIGPIPE when writing to a closed client
#ifdef SO_NOSIGPIPE
        int nosig = 1;
        setsockopt(client_fd, SOL_SOCKET, SO_NOSIGPIPE, &nosig, sizeof(nosig));
#endif
        if (doHandshake(client_fd)) {
          std::lock_guard<std::mutex> lock(clients_mutex_);
          clients_.push_back(client_fd);
          std::cout << "WebSocket: client connected (" << clients_.size()
                    << " total)\n";
        } else {
          ::close(client_fd);
        }
      }
    }
  }

  ::close(server_fd_);
  server_fd_ = -1;
}

void WebsocketServer::stop() {
  running_ = false;
  if (server_fd_ >= 0) ::shutdown(server_fd_, SHUT_RDWR);
  std::lock_guard<std::mutex> lock(clients_mutex_);
  for (int fd : clients_) ::close(fd);
  clients_.clear();
}

size_t WebsocketServer::numConnections() {
  std::lock_guard<std::mutex> lock(clients_mutex_);
  return clients_.size();
}

bool WebsocketServer::doHandshake(int fd) {
  char buf[4096] = {};
  int n = static_cast<int>(::recv(fd, buf, sizeof(buf) - 1, 0));
  if (n <= 0) return false;

  std::string req(buf, n);
  const std::string key_header = "Sec-WebSocket-Key: ";
  auto pos = req.find(key_header);
  if (pos == std::string::npos) return false;
  pos += key_header.size();
  auto end = req.find("\r\n", pos);
  if (end == std::string::npos) return false;
  std::string key = req.substr(pos, end - pos);

  std::string response =
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: " +
      ws_accept_key(key) + "\r\n\r\n";

  return ::send(fd, response.data(), response.size(), 0) ==
         static_cast<ssize_t>(response.size());
}

bool WebsocketServer::sendFrame(int fd, const void* data, size_t len) {
  uint8_t header[10];
  int header_len;

  header[0] = 0x82;  // FIN=1, opcode=2 (binary)
  if (len <= 125) {
    header[1] = static_cast<uint8_t>(len);
    header_len = 2;
  } else if (len <= 65535) {
    header[1] = 126;
    header[2] = (len >> 8) & 0xFF;
    header[3] = len & 0xFF;
    header_len = 4;
  } else {
    header[1] = 127;
    for (int i = 0; i < 8; ++i)
      header[2 + i] = static_cast<uint8_t>(len >> (56 - 8 * i));
    header_len = 10;
  }

#ifdef MSG_NOSIGNAL
  const int flags = MSG_NOSIGNAL;
#else
  const int flags = 0;
#endif

  if (::send(fd, header, header_len, flags) != header_len) return false;
  if (::send(fd, data, len, flags) != static_cast<ssize_t>(len)) return false;
  return true;
}

void WebsocketServer::sendData(const float* data, size_t byte_len) {
  std::lock_guard<std::mutex> lock(clients_mutex_);
  if (clients_.empty()) return;

  auto it = clients_.begin();
  while (it != clients_.end()) {
    if (sendFrame(*it, data, byte_len)) {
      ++it;
    } else {
      std::cout << "WebSocket: client disconnected\n";
      ::close(*it);
      it = clients_.erase(it);
    }
  }
}
