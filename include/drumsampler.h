#pragma once

#include "soundgenerator.h"
#include "envelope_generator.h"
#include "filter_moogladder.h"
#include "stereodelay.h"

#include <sndfile.h>
#include <stdbool.h>
#include <wchar.h>

#define DEFAULT_AMP 0.7
#define MAX_CONCURRENT_SAMPLES 10 // arbitrary

typedef struct sample_pos
{
    int position;
    int playing;
    int played;
    double audiobuffer_cur_pos;
    double audiobuffer_inc;
    double audiobuffer_pitch;
    double amp;
    double speed;
    double start_pos_pct;
    double end_pos_pct;
} sample_pos;

class drumsampler : public SoundGenerator
{
  public:
    drumsampler(char *filename);
    ~drumsampler();
    void status(wchar_t *status_string) override;
    stereo_val genNext() override;
    void start() override;
    void noteOn(midi_event ev) override;
    void SetParam(std::string name, double val) override;
    double GetParam(std::string name) override;

  public:
    bool glitch_mode;
    int glitch_rand_factor;

    sample_pos sample_positions[PPBAR];
    int samples_now_playing[MAX_CONCURRENT_SAMPLES]; // contains midi tick of
                                                     // current samples
    int velocity_now_playing[MAX_CONCURRENT_SAMPLES];

    char filename[1024];
    int samplerate;
    int channels;

    envelope_generator eg;
    bool envelope_enabled;

    double *buffer;
    int bufsize;
    int buf_end_pos; // this will always be shorter than bufsize for cutting off
                     // sample earlier
    double buffer_pitch;
    // int buf_num_channels;

    int swing;
    bool started; // to sync at top of loop
};

int get_a_drumsampler_position(drumsampler *ss);
void drumsampler_import_file(drumsampler *s, char *filename);
void drumsampler_reset_samples(drumsampler *seq);
void drumsampler_set_pitch(drumsampler *seq, double v);
void drumsampler_set_cutoff_percent(drumsampler *seq, unsigned int percent);
void drumsampler_enable_envelope_generator(drumsampler *ds, bool b);
void drumsampler_set_attack_time(drumsampler *ds, double val);
void drumsampler_set_decay_time(drumsampler *ds, double val);
void drumsampler_set_sustain_lvl(drumsampler *ds, double val);
void drumsampler_set_release_time(drumsampler *ds, double val);
void drumsampler_set_glitch_mode(drumsampler *ds, bool b);
void drumsampler_set_glitch_rand_factor(drumsampler *ds, int pct);
