#ifndef SAMPLER_H
#define SAMPLER_H

#include "sound_generator.h"
#include <pthread.h>
#include <stdbool.h>
#include <wchar.h>

// TODO use this more - at the moment just for the int array
#define MAX_SAMPLES_PER_LOOPER 10

typedef struct file_sample {
    char *filename;
    int *orig_file_bytes;
    int orig_file_size;

    double *resampled_file_bytes;
    int resampled_file_size;

    int position;

    int samplerate;
    int channels;

    int loop_len;
} file_sample;

typedef struct t_sampler {
    SOUNDGEN sound_generator;

    file_sample *samples[MAX_SAMPLES_PER_LOOPER];
    int sample_num_loops[MAX_SAMPLES_PER_LOOPER];
    file_sample *scramblrrr; // for storing scrambled effect version of loop
    int num_samples;
    int cur_sample;
    int cur_sample_iteration;
    bool multi_sample_mode;
    // bool multi_sample_loop_countdown_started;

    pthread_mutex_t resample_mutex;

    double vol;
    bool started;
    bool just_been_resampled;

    bool scramblrrr_mode;
    int scramble_counter;
    int scramble_generation;

    bool stutter_mode;
    int stutter_current_16th;
    int stutter_generation;

    int max_generation;

} SAMPLER;

SAMPLER *new_sampler(char *filename, double loop_len); // loop_len in bars
void sampler_add_sample(SAMPLER *s, char *filename, int loop_len);
file_sample *sampler_create_sample(char *filename, int loop_len);

void sampler_scramble(SAMPLER *s);
void sampler_set_scramble_mode(SAMPLER *s, bool b);
void sampler_set_max_generation(SAMPLER *s, int max);

void sampler_set_stutter_mode(SAMPLER *s, bool b);

void sampler_set_multi_sample_mode(SAMPLER *s, bool multi);
void sampler_switch_sample(SAMPLER *s, int sample_num);
void sampler_change_loop_len(SAMPLER *s, int sample_num, int loop_len);
void sampler_change_num_loops(SAMPLER *s, int sample_num, int num_loops);
// void sampler_gennext(void* self, double* frame_vals, int framesPerBuffer);
double sampler_gennext(void *self);

void sampler_status(void *self, wchar_t *ss);
void sampler_setvol(void *self, double v);
double sampler_getvol(void *self);

void sample_import_file_contents(file_sample *fs, char *filename);
void sample_set_file_name(file_sample *fs, char *filename);
void sample_resample_to_loop_size(file_sample *fs);

#if defined(__cplusplus)
extern "C" {
#endif
void sampler_resample_to_loop_size(SAMPLER *s);
#if defined(__cplusplus)
}
#endif

#endif // SAMPLER_H
