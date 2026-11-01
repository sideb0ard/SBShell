#pragma once

#include <complex>
#include <vector>

void FFT(std::vector<std::complex<double>>& x);
void iFFT(std::vector<std::complex<double>>& x);

// Condenses FFT magnitudes (in dB, bins 0..N/2) into num_bands log-spaced
// bands, each 0.0-1.0: floor_db and below -> 0, 0 dB -> 1. Each band takes
// the loudest bin in its range. DC (bin 0) is skipped, and every band gets at
// least one bin, so the lowest bands are one bin wide at small FFT sizes.
std::vector<double> CondenseToBands(const std::vector<double>& bins_db,
                                    int num_bands, double floor_db = -60.0);
