#pragma once

// Pure POSIX socket WebSocket server — no ASIO, no WebSocketPP.
// Avoids the link_asio_1_30_2 namespace clash with Ableton Link.

#include <atomic>
#include <mutex>
#include <vector>

class WebsocketServer {
 public:
  WebsocketServer() = default;
  ~WebsocketServer();

  // Blocking — call from a dedicated thread. Returns when stop() is called.
  void run(int port);
  void stop();

  // Send raw float audio data as a binary WebSocket frame to all clients.
  // byte_len is the number of bytes (not floats).
  void sendData(const float* data, size_t byte_len);

  size_t numConnections();

 private:
  bool doHandshake(int client_fd);
  bool sendFrame(int fd, const void* data, size_t len);

  int server_fd_{-1};
  std::vector<int> clients_;
  std::mutex clients_mutex_;
  std::atomic<bool> running_{false};
};
