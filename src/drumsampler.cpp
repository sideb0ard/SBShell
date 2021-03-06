#include <libgen.h>
#include <math.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "defjams.h"
#include "drumsampler.h"
#include "mixer.h"
#include "utils.h"

#include <iomanip>
#include <iostream>

extern wchar_t *sparkchars;
extern mixer *mixr;

DrumSampler::DrumSampler(char *filename)
{
    drumsampler_import_file(this, filename);

    envelope_enabled = false;
    glitch_mode = false;
    glitch_rand_factor = 20;

    type = DRUMSAMPLER_TYPE;

    envelope_generator_init(&eg);
    eg.m_sustain_level = 0;

    active = true;
    started = false;
}

DrumSampler::~DrumSampler()
{
    printf("Deleting sample buffer\n");
    free(buffer);
}

stereo_val DrumSampler::genNext()
{
    double left_val = 0;
    double right_val = 0;

    for (int i = 0; i < MAX_CONCURRENT_SAMPLES; i++)
    {
        if (samples_now_playing[i] != -1)
        {
            int cur_sample_midi_tick = samples_now_playing[i];
            int velocity = velocity_now_playing[i];
            double amp = scaleybum(0, 127, 0, 1, velocity);
            int idx =
                sample_positions[cur_sample_midi_tick].audiobuffer_cur_pos;
            left_val += buffer[idx] * amp;

            if (channels == 2)
                right_val += buffer[idx + 1] * amp;
            else
                right_val = left_val;

            if (glitch_mode)
            {
                if (sample_positions[cur_sample_midi_tick].audiobuffer_cur_pos >
                        (bufsize / 2) &&
                    rand() % 100 > glitch_rand_factor)
                {
                    continue;
                }
            }

            sample_positions[cur_sample_midi_tick].audiobuffer_cur_pos =
                sample_positions[cur_sample_midi_tick].audiobuffer_cur_pos +
                (channels * (buffer_pitch));

            if ((int)sample_positions[cur_sample_midi_tick]
                    .audiobuffer_cur_pos >= buf_end_pos)
            { // end of playback - so reset
                samples_now_playing[i] = -1;
                sample_positions[cur_sample_midi_tick].audiobuffer_cur_pos = 0;
            }
        }
    }

    double amp_env = eg_do_envelope(&eg, NULL);
    if (eg.m_state == SUSTAIN)
        eg_note_off(&eg);

    double amp = volume;
    if (envelope_enabled)
        amp *= amp_env;

    pan = fmin(pan, 1.0);
    pan = fmax(pan, -1.0);
    double pan_left = 0.707;
    double pan_right = 0.707;
    calculate_pan_values(pan, &pan_left, &pan_right);

    stereo_val out = {.left = left_val * amp * pan_left,
                      .right = right_val * amp * pan_right};
    out = Effector(out);
    return out;
}

std::string DrumSampler::Status()
{
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    if (!active || volume == 0)
        ss << ANSI_COLOR_RESET;
    else
        ss << COOL_COLOR_PINK;

    ss << "Sampler(" << filename << ")"
       << " vol:" << volume << " pan:" << pan << " pitch:" << buffer_pitch
       << ANSI_COLOR_RESET;

    return ss.str();
}

std::string DrumSampler::Info()
{
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    ss << ANSI_COLOR_BLUE << "\nSample ";
    ss << ANSI_COLOR_WHITE << filename << ANSI_COLOR_BLUE;
    ss << "\nvol:" << volume << " pan:" << pan << " pitch:" << buffer_pitch;
    ss << "eg:" << envelope_enabled << " attack_ms:" << eg.m_attack_time_msec;
    ss << " decay_ms:" << eg.m_decay_time_msec
       << " sustain: " << eg.m_sustain_level;
    return ss.str();
}

void DrumSampler::start()
{
    if (active)
        return; // no-op
    drumsampler_reset_samples(this);
    active = true;
    engine.cur_step = mixr->timing_info.sixteenth_note_tick % 16;
}

void DrumSampler::noteOn(midi_event ev)
{
    int idx = mixr->timing_info.midi_tick % PPBAR;
    int seq_position = get_a_drumsampler_position(this);
    if (seq_position != -1)
    {
        samples_now_playing[seq_position] = idx;
        velocity_now_playing[seq_position] = ev.data2;
    }
    eg_start_eg(&eg);
}

void DrumSampler::pitchBend(midi_event ev)
{
    float pitch_val = ev.data1 / 10.;
    drumsampler_set_pitch(this, pitch_val);
}

void drumsampler_import_file(DrumSampler *ds, char *filename)
{
    audio_buffer_details deetz = import_file_contents(&ds->buffer, filename);
    strcpy(ds->filename, deetz.filename);
    ds->bufsize = deetz.buffer_length;
    ds->buf_end_pos = ds->bufsize;
    ds->buffer_pitch = 1.0;
    ds->samplerate = deetz.sample_rate;
    ds->channels = deetz.num_channels;
    drumsampler_reset_samples(ds);
}

void drumsampler_reset_samples(DrumSampler *ds)
{
    for (int i = 0; i < MAX_CONCURRENT_SAMPLES; i++)
    {
        ds->samples_now_playing[i] = -1;
    }
    for (int i = 0; i < PPBAR; i++)
    {
        ds->sample_positions[i].position = 0;
        ds->sample_positions[i].audiobuffer_cur_pos = 0.;
        ds->sample_positions[i].audiobuffer_inc = 1.0;
        ds->sample_positions[i].playing = 0;
        ds->sample_positions[i].played = 0;
        ds->sample_positions[i].amp = 0;
        ds->sample_positions[i].speed = 1;
    }
}

int get_a_drumsampler_position(DrumSampler *ds)
{
    for (int i = 0; i < MAX_CONCURRENT_SAMPLES; i++)
        if (ds->samples_now_playing[i] == -1)
            return i;
    return -1;
}

void drumsampler_set_pitch(DrumSampler *ds, double v)
{
    if (v >= 0. && v <= 2.0)
        ds->buffer_pitch = v;
    else
        printf("Must be in the range of 0.0 .. 2.0\n");
}

void drumsampler_set_cutoff_percent(DrumSampler *ds, unsigned int percent)
{
    if (percent > 100)
        return;
    ds->buf_end_pos = ds->bufsize / 100. * percent;
}

void drumsampler_enable_envelope_generator(DrumSampler *ds, bool b)
{
    ds->envelope_enabled = b;
}
void drumsampler_set_attack_time(DrumSampler *ds, double val)
{
    eg_set_attack_time_msec(&ds->eg, val);
}
void drumsampler_set_decay_time(DrumSampler *ds, double val)
{
    eg_set_decay_time_msec(&ds->eg, val);
}
void drumsampler_set_sustain_lvl(DrumSampler *ds, double val)
{
    eg_set_sustain_level(&ds->eg, val);
}
void drumsampler_set_release_time(DrumSampler *ds, double val)
{
    eg_set_release_time_msec(&ds->eg, val);
}

void drumsampler_set_glitch_mode(DrumSampler *ds, bool b)
{
    ds->glitch_mode = b;
}

void drumsampler_set_glitch_rand_factor(DrumSampler *ds, int pct)
{
    if (pct >= 0 && pct <= 100)
        ds->glitch_rand_factor = pct;
}
void DrumSampler::SetParam(std::string name, double val)
{
    if (name == "pitch")
        drumsampler_set_pitch(this, val);
}
double DrumSampler::GetParam(std::string name) { return 0; }
