#ifndef MIXER_H
#define MIXER_H

#include <portaudio.h>

#include "drumr.h"
#include "effect.h"
#include "minisynth.h"
#include "sbmsg.h"
#include "sound_generator.h"
#include "table.h"

#include "ableton_link_wrapper.h"

#define MAX_PS_STRING_SZ 512 // arbitrary

#define ENVIRONMENT_ARRAY_SIZE 128
#define ENVIRONMENT_KEY_SIZE 128

typedef struct environment_variable {
    char key[ENVIRONMENT_KEY_SIZE];
    int val;
} env_var;

typedef enum {
    MIDI_MODE_ONE,
    MIDI_MODE_TWO,
    MAX_NUM_MIDI_MODES,
} midi_mode; // to switch control knob routing

typedef struct t_mixer {

    SOUNDGEN **sound_generators;
    int soundgen_num;  // actual number of SGs
    int soundgen_size; // number of memory slots reserved for SGszz

    env_var environment[ENVIRONMENT_ARRAY_SIZE];
    int env_var_count;

    int delay_on;
    bool debug_mode;

    bool m_is_playing;

    EFFECT **effects;
    int effects_num;
    int effects_size;

    unsigned int midi_control_destination;
    unsigned int m_midi_controller_mode; // to switch control knob routing

    int active_midi_soundgen_num;
    int active_midi_soundgen_effect_num;

    // double bpm;

    //  int samples_per_midi_tick;
    //  int midi_ticks_per_ms;
    int sixteenth_note_tick;
    bool start_of_sixteenth;
    // int cur_sample; // inverse of SAMPLE RATE
    int tick; //

    // informational for other sound generators
    //  unsigned int loop_len_in_samples;
    //  unsigned int loop_len_in_ticks;

    double volume;
    int keyboard_octave; // TODO <- need this?

    AbletonLink *m_ableton_link;

} mixer;

typedef struct {
    mixer *mixr;
} paData;

mixer *new_mixer(void);

void mixer_ps(mixer *mixr);
void mixer_update_bpm(mixer *mixr, int bpm);
int add_algorithm(char *line);
int add_bitwize(mixer *mixr, int pattern);
int add_bytebeat(mixer *mixr, char *pattern);
int add_chaosmonkey(void);
int add_minisynth(mixer *mixr);
int add_drum_char_pattern(mixer *mixr, char *filename, char *pattern);
int add_drum_euclidean(mixer *mixr, char *filename, int num_beats,
                       bool start_on_first_beat);
int add_sampler(mixer *mixr, char *filename, double loop_len);
int add_sound_generator(mixer *mixr, SBMSG *sbm);
int add_effect(mixer *mixr);
void mixer_vol_change(mixer *mixr, float vol);
void vol_change(mixer *mixr, int sig, float vol);
void freq_change(mixer *mixr, int sig, float freq);
void delay_toggle(mixer *mixr);

void mixer_toggle_midi_mode(mixer *mixr);
void mixer_start_playing(mixer *mixr);
void mixer_stop_playing(mixer *mixr);

void update_environment(char *key, int val);
int get_environment_val(char *key, int *return_val);

double gen_next(mixer *mixr, uint64_t host_time, int sample_number);
// void gen_next(mixer* mixr, int framesPerBuffer, float* out);

#endif // MIXER_H
