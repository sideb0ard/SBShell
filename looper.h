#ifndef LOOPER_H
#define LOOPER_H

#include "filter_moogladder.h"
#include "sound_generator.h"
#include "stereodelay.h"
#include <stdbool.h>
#include <wchar.h>

#define MAX_SAMPLES_PER_LOOPER 10
#define MAX_SOUND_GRAINS 1000

typedef struct file_sample {
    char *filename;
    int *orig_file_bytes;
    int orig_file_size;
    double *resampled_file_bytes;
    int resampled_file_size;
    int position;
    int samplerate;
    int channels;
    int frames;
    int loop_len;
} file_sample;

#define MAX_GRAIN_STREAM_LEN_SEC 10

typedef struct sound_grain {
    int grain_duration_ms;
    int grain_len_samples;
    int audiobuffer_num;
    int audiobuffer_idx;
    bool active;
} sound_grain;

enum {
    GRAIN_SEQUENTIAL,
    GRAIN_RANDOM,
    GRAIN_REVERSED,
    GRAIN_NUM_SELECTION_MODES
};

typedef struct t_looper {
    SOUNDGEN sound_generator;

    file_sample *samples[MAX_SAMPLES_PER_LOOPER];
    int sample_num_loops[MAX_SAMPLES_PER_LOOPER];
    file_sample *scramblrrr; // for storing scrambled effect version of loop
    int loop_len;
    int num_samples;
    int cur_sample;
    int cur_sample_iteration;
    bool multi_sample_mode;
    // bool multi_sample_loop_countdown_started;

    bool active;
    bool started;
    bool just_been_resampled;

    bool granulate_mode;
    int grain_duration;
    int grains_per_sec;
    unsigned int grain_selection;
    sound_grain m_grains[MAX_SOUND_GRAINS];
    int grain_stream[MAX_GRAIN_STREAM_LEN_SEC * SAMPLE_RATE];
    int grain_stream_read_idx;

    bool scramblrrr_mode;
    bool scramblrrr_active;
    int scramble_counter;
    int scramble_every_n_loops;
    int scramble_every_n_loops_generation;
    int scramble_generation;

    bool stutter_mode;
    bool stutter_active;
    int stutter_every_n_loops;
    int stutter_every_n_loops_generation;
    int stutter_current_16th;
    int stutter_generation;

    int max_generation;

    double vol;

    bool resample_pending;
    bool change_loopsize_pending;
    int pending_loop_size;
    int pending_loop_num;

} looper;

looper *new_looper(char *filename, double loop_len); // loop_len in bars
void looper_add_sample(looper *s, char *filename, int loop_len);
file_sample *looper_create_sample(char *filename, int loop_len);

void looper_scramble(looper *s);
void looper_set_scramble_mode(looper *s, bool b);
void looper_set_max_generation(looper *s, int max);

void looper_set_stutter_mode(looper *s, bool b);

void looper_set_multi_sample_mode(looper *s, bool multi);
void looper_switch_sample(looper *s, int sample_num);
void looper_resample_to_loop_size(looper *s);
void looper_change_loop_len(looper *s, int sample_num, int loop_len);
void looper_change_num_loops(looper *s, int sample_num, int num_loops);
// void looper_gennext(void* self, double* frame_vals, int framesPerBuffer);
double looper_gennext(void *self);

void looper_status(void *self, wchar_t *ss);
void looper_setvol(void *self, double v);
double looper_getvol(void *self);
void looper_start(void *self);
void looper_stop(void *self);
void looper_make_active_track(void *self, int track_num);
int looper_get_num_tracks(void *self);

void looper_refresh_grain_stream(looper *l);
void looper_set_granulate(looper *l, bool b);
void looper_set_grain_duration(looper *l, int dur);
void looper_set_grains_per_sec(looper *l, int gps);
void looper_set_grain_selection_mode(looper *l, unsigned int mode);

void sample_import_file_contents(file_sample *fs, char *filename);
void sample_set_file_name(file_sample *fs, char *filename);
void sample_resample_to_loop_size(file_sample *fs);

void file_sample_free(file_sample *fs);
void looper_del_self(looper *s);

#endif // LOOPER_H
