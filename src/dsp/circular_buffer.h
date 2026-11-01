#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

// Fixed-capacity ring buffer. Storage is inline (std::array), so it never
// allocates and is safe to push to from the audio thread. Once full, each
// push_back overwrites the oldest value.
template <typename T, size_t Capacity>
class CircularBuffer {
  static_assert(Capacity > 0, "CircularBuffer capacity must be > 0");

 public:
  void push_back(const T& val) {
    buffer[tail] = val;
    tail = (tail + 1) % Capacity;
    if (size < Capacity) {
      size++;
    } else {
      head = (head + 1) % Capacity;
    }
  }

  T& front() {
    return buffer[head];
  }

  T& back() {
    return buffer[(tail + Capacity - 1) % Capacity];
  }

  size_t get_size() const {
    return size;
  }

  static constexpr size_t capacity() {
    return Capacity;
  }

  bool empty() const {
    return size == 0;
  }

  bool full() const {
    return size == Capacity;
  }

  // Copies the newest n values (oldest first) into out. Returns how many were
  // copied (fewer than n if the buffer isn't that full yet).
  size_t copy_last(T* out, size_t n) const {
    n = std::min(n, size);
    size_t start = (tail + Capacity - n) % Capacity;
    for (size_t i = 0; i < n; i++) {
      out[i] = buffer[(start + i) % Capacity];
    }
    return n;
  }

 private:
  std::array<T, Capacity> buffer{};
  size_t head{0};
  size_t tail{0};
  size_t size{0};
};
