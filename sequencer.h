#pragma once

#include <stdbool.h>
#include <wchar.h>

#define GRIDWIDTH (SEQUENCER_PATTERN_LEN / 4)
#define INTEGER_LENGTH pow(2, SEQUENCER_PATTERN_LEN)
#define NUM_SEQUENCER_PATTERNS 10

typedef enum { MARKOVHAUS, MARKOVBOOMBAP } markovmodez;

typedef struct sequencer {

    int tick;

    int matrix1[GRIDWIDTH][GRIDWIDTH];
    int matrix2[GRIDWIDTH][GRIDWIDTH];

    int patterns[NUM_SEQUENCER_PATTERNS];
    double pattern_position_amp[NUM_SEQUENCER_PATTERNS][SEQUENCER_PATTERN_LEN];
    int pattern_num_loops[NUM_SEQUENCER_PATTERNS];
    int backup_pattern_while_getting_crazy; // store current pattern so
                                            // algorithms can use slot
    int num_patterns;
    int cur_pattern;
    int cur_pattern_iteration;
    bool multi_pattern_mode;

    bool game_of_life_on;
    int life_generation;
    int life_every_n_loops;

    bool markov_on;
    unsigned int markov_mode; // MARKOVHAUS or MARKOVBOOMBAP
    int markov_generation;
    int markov_every_n_loops;

    bool bitwise_on;
    unsigned int bitwise_mode;
    int bitwise_generation;
    int bitwise_every_n_loops;

    int max_generation; // used for game of life, markov chain and bitwise

} sequencer;

void seq_init(sequencer *s);
bool seq_tick(sequencer *s);
void seq_status(sequencer *s, wchar_t *status_string);

void seq_set_sample_amp_from_char_pattern(sequencer *s, int pattern_num,
                                          char *amp_pattern);
void seq_set_random_sample_amp(sequencer *s, int pattern_num);
void add_char_pattern(sequencer *s, char *pattern);
void change_char_pattern(sequencer *s, int pattern_num, char *pattern);
void add_int_pattern(sequencer *s, int pattern);
void change_int_pattern(sequencer *s, int pattern_num, int pattern);

void seq_set_multi_pattern_mode(sequencer *s, bool multi);
void seq_change_num_loops(sequencer *s, int pattern_num, int num_loops);

void int_pattern_to_array(int pattern, int *pat_array);
void pattern_char_to_int(char *chpattern, int *pattern);
void wchar_version_of_amp(sequencer *s, int pattern_num, wchar_t apattern[49]);

int seed_pattern(void);
int matrix_to_int(int matrix[GRIDWIDTH][GRIDWIDTH]);
void int_to_matrix(int pattern, int matrix[GRIDWIDTH][GRIDWIDTH]);
void next_life_generation(sequencer *s);
void next_markov_generation(sequencer *s);

void seq_set_game_of_life(sequencer *s, bool on);
void seq_set_markov(sequencer *s, bool on);
void seq_set_markov_mode(sequencer *s, unsigned int mode);
void seq_set_bitwise(sequencer *s, bool on);
void seq_set_bitwise_mode(sequencer *s, unsigned int mode);
void seq_set_backup_mode(sequencer *s, bool on);
void seq_set_max_generations(sequencer *s, int max);