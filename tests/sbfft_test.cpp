#include "dsp/sbfft.h"

#include <cmath>
#include <complex>
#include <vector>

#include "gtest/gtest.h"

namespace {

constexpr int kBins = 257;  // N/2 + 1 for a 512-point FFT

TEST(SbFftTest, SinePeaksAtItsBin) {
  const int N = 64;
  const int k = 8;
  std::vector<std::complex<double>> x(N);
  for (int n = 0; n < N; n++) x[n] = std::sin(2.0 * M_PI * k * n / N);
  FFT(x);

  int peak = 0;
  for (int i = 1; i <= N / 2; i++)
    if (std::abs(x[i]) > std::abs(x[peak])) peak = i;
  EXPECT_EQ(peak, k);
  EXPECT_NEAR(std::abs(x[k]), N / 2.0, 1e-9);
}

TEST(SbFftTest, IfftUndoesFft) {
  std::vector<std::complex<double>> x = {1, -2, 3, 0.5, 0, 7, -1, 2};
  auto y = x;
  FFT(y);
  iFFT(y);
  for (size_t i = 0; i < x.size(); i++)
    EXPECT_NEAR(std::abs(y[i] - x[i]), 0, 1e-9);
}

TEST(SbFftTest, CondenseReturnsRequestedBandCount) {
  std::vector<double> silent(kBins, -200.0);
  for (int n : {8, 16, 32}) {
    auto bands = CondenseToBands(silent, n);
    ASSERT_EQ(bands.size(), static_cast<size_t>(n));
    for (double b : bands) EXPECT_EQ(b, 0.0);
  }
}

TEST(SbFftTest, CondenseMapsDbToZeroOne) {
  std::vector<double> bins(kBins, -200.0);
  bins[kBins - 1] = 0.0;  // full scale at Nyquist -> top band
  bins[1] = -30.0;        // half way down the -60 dB range -> bottom band
  auto bands = CondenseToBands(bins, 8);
  EXPECT_DOUBLE_EQ(bands.back(), 1.0);
  EXPECT_DOUBLE_EQ(bands.front(), 0.5);
}

TEST(SbFftTest, CondenseIgnoresDc) {
  std::vector<double> bins(kBins, -200.0);
  bins[0] = 0.0;
  for (double b : CondenseToBands(bins, 16)) EXPECT_EQ(b, 0.0);
}

// Every non-DC bin lands in exactly one band, and bands go low to high.
TEST(SbFftTest, CondenseCoversEveryBinOnceInOrder) {
  for (int n : {8, 16, 32}) {
    int last_band = 0;
    for (int k = 1; k < kBins; k++) {
      std::vector<double> bins(kBins, -200.0);
      bins[k] = 0.0;
      auto bands = CondenseToBands(bins, n);
      int hits = 0, band = -1;
      for (int b = 0; b < n; b++)
        if (bands[b] == 1.0) {
          hits++;
          band = b;
        }
      ASSERT_EQ(hits, 1) << "bands=" << n << " bin=" << k;
      EXPECT_GE(band, last_band) << "bands=" << n << " bin=" << k;
      last_band = band;
    }
    EXPECT_EQ(last_band, n - 1);
  }
}

TEST(SbFftTest, CondenseRejectsBadInput) {
  EXPECT_TRUE(CondenseToBands({}, 16).empty());
  EXPECT_TRUE(CondenseToBands(std::vector<double>(kBins, 0.0), 0).empty());
}

}  // namespace
