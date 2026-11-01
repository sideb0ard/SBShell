#include "sbfft.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

void FFT(std::vector<std::complex<double>>& x) {
  const size_t N = x.size();
  if (N <= 1) return;

  std::vector<std::complex<double>> even(N / 2);
  std::vector<std::complex<double>> odd(N / 2);
  for (size_t i = 0; i < N / 2; ++i) {
    even[i] = x[2 * i];
    odd[i] = x[2 * i + 1];
  }
  FFT(even);
  FFT(odd);

  for (size_t k = 0; k < N / 2; k++) {
    std::complex<double> twiddle =
        std::polar(1.0, -2.0 * M_PI * k / N) * odd[k];
    x[k] = even[k] + twiddle;
    x[k + N / 2] = even[k] - twiddle;
  }
}

void iFFT(std::vector<std::complex<double>>& x) {
  const size_t n = x.size();

  // conjugate input
  for (auto& v : x) v = std::conj(v);

  FFT(x);

  // conjugate result and normalize ny N
  for (auto& v : x) v = std::conj(v) / static_cast<double>(n);
}

std::vector<double> CondenseToBands(const std::vector<double>& bins_db,
                                    int num_bands, double floor_db) {
  const int last_bin = static_cast<int>(bins_db.size()) - 1;  // Nyquist
  if (num_bands <= 0 || last_bin < 1 || floor_db >= 0) return {};

  std::vector<double> bands(num_bands, 0.0);
  int lo = 1;  // skip DC
  for (int b = 0; b < num_bands; b++) {
    // Exclusive end of this band, log-spaced between bin 1 and Nyquist
    int hi = (b == num_bands - 1)
                 ? last_bin + 1
                 : static_cast<int>(std::lround(
                       std::pow(static_cast<double>(last_bin),
                                static_cast<double>(b + 1) / num_bands)));
    hi = std::min(std::max(hi, lo + 1), last_bin + 1);
    if (lo >= hi) break;  // more bands than bins: the rest stay 0

    double peak_db = floor_db;
    for (int k = lo; k < hi; k++) peak_db = std::max(peak_db, bins_db[k]);
    bands[b] = std::clamp((peak_db - floor_db) / -floor_db, 0.0, 1.0);
    lo = hi;
  }
  return bands;
}
