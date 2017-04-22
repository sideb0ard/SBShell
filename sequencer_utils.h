#pragma once

#include <wchar.h>

// https://ics-web.sns.ornl.gov/timing/Rep-Rate%20Tech%20Note.pdf
void build_euclidean_pattern_int(int level, int *bitmap_int,
                                 int *bitmap_position, int *count,
                                 int *remainderrr);
int create_euclidean_rhythm(int num_beats, int len_pattern);

int shift_bits_to_leftmost_position(int num, int num_of_bits_to_align_with);
void char_binary_version_of_int(int num, char bin_num[17]);
void wchar_binary_version_of_int(int num, wchar_t bin_num[17]);
unsigned int gimme_a_bitwise_int(int pattern, int t);