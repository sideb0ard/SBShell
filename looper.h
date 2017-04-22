#ifndef LOOPER_H
#define LOOPER_H

#include "filter_moogladder.h"
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

typedef struct t_looper {
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

    bool started;
    bool just_been_resampled;

    bool scramblrrr_mode;
    int scramble_counter;
    int scramble_generation;

    bool stutter_mode;
    int stutter_current_16th;
    int stutter_generation;

    int max_generation;

    stereodelay m_delay_fx;
    // midi - top row
    double m_fc_control;
    double m_q_control;
    int swing_setting;
    double vol;
    filter_moogladder m_filter;
    // midi - bottom row, mode 1
    double m_delay_time_msec;
    double m_feedback_pct;
    double m_delay_ratio;
    double m_wet_mix;
    unsigned int m_delay_mode; // pad5 button

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

void sample_import_file_contents(file_sample *fs, char *filename);
void sample_set_file_name(file_sample *fs, char *filename);
void sample_resample_to_loop_size(file_sample *fs);

void looper_parse_midi(looper *s, unsigned int data1, unsigned int data2);

#endif // LOOPER_H