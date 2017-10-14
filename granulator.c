#include <libgen.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defjams.h"
#include "granulator.h"
#include "mixer.h"
#include "utils.h"

extern mixer *mixr;
extern char *s_lfo_mode_names;

granulator *new_granulator(char *filename)
{
    granulator *g = (granulator *)calloc(1, sizeof(granulator));
    g->vol = 0.7;
    // g->active = true;
    g->started = false;
    g->have_active_buffer = false;

    g->grain_buffer_position = 0;
    g->granular_spray = 441; // 10 ms * 44.1
    g->grain_duration_ms = 50;
    g->grains_per_sec = 30;
    g->grain_attack_time_pct = 20;
    g->grain_release_time_pct = 20;
    g->quasi_grain_fudge = 220;
    g->selection_mode = GRAIN_SELECTION_STATIC;
    g->external_source_sg = -1;

    g->grain_pitch = 1;
    g->sequencer_mode = false;

    g->sound_generator.gennext = &granulator_gennext;
    g->sound_generator.status = &granulator_status;
    g->sound_generator.getvol = &granulator_getvol;
    g->sound_generator.setvol = &granulator_setvol;
    g->sound_generator.start = &granulator_start;
    g->sound_generator.stop = &granulator_stop;
    g->sound_generator.get_num_tracks = &granulator_get_num_tracks;
    g->sound_generator.make_active_track = &granulator_make_active_track;
    g->sound_generator.self_destruct = &granulator_del_self;
    g->sound_generator.type = GRANULATOR_TYPE;

    if (strncmp(filename, "none", 4) != 0)
        granulator_import_file(g, filename);

    seq_init(&g->m_seq);
    granulator_set_sequencer_mode(g, false);

    envelope_generator_init(&g->m_eg1); // start/stop env
    g->m_eg1.m_attack_time_msec = 1000;
    g->m_eg1.m_release_time_msec = 750;

    g->graindur_lfo_on = false;
    g->m_lfo1_min = 20;
    g->m_lfo1_max = 80;
    osc_new_settings((oscillator *)&g->m_lfo1);
    lfo_set_soundgenerator_interface(&g->m_lfo1);
    g->m_lfo1.osc.m_osc_fo = 0.01; // default LFO
    g->m_lfo1.osc.m_amplitude = 1.;
    g->lfo1_sync = false;
    lfo_start_oscillator((oscillator *)&g->m_lfo1);

    g->grainps_lfo_on = false;
    g->m_lfo2_min = 20;
    g->m_lfo2_max = 90;
    osc_new_settings((oscillator *)&g->m_lfo2);
    lfo_set_soundgenerator_interface(&g->m_lfo2);
    g->m_lfo2.osc.m_osc_fo = 0.01; // default LFO
    g->m_lfo2.osc.m_amplitude = 1.;
    g->lfo2_sync = false;
    lfo_start_oscillator((oscillator *)&g->m_lfo2);

    g->grainscanfile_lfo_on = false;
    g->m_lfo3_min = 0;
    g->m_lfo3_max = g->audio_buffer_len;
    osc_new_settings((oscillator *)&g->m_lfo3);
    lfo_set_soundgenerator_interface(&g->m_lfo3);
    g->m_lfo3.osc.m_osc_fo = 0.01; // default LFO
    g->m_lfo3.osc.m_amplitude = 1.;
    g->lfo3_sync = false;
    lfo_start_oscillator((oscillator *)&g->m_lfo3);

    granulator_start(g);

    return g;
}

stereo_val granulator_gennext(void *self)
{
    granulator *g = (granulator *)self;
    double val = 0;

    if (g->external_source_sg != -1)
    {
        g->audio_buffer[g->audio_buffer_write_idx] =
            mixr->soundgen_cur_val[g->external_source_sg].left;
        g->audio_buffer_write_idx++;
        if (g->audio_buffer_write_idx >= g->audio_buffer_len)
            g->audio_buffer_write_idx = 0;
    }

    if (g->sequencer_mode && g->m_seq.num_patterns > 0)
    {
        int idx = mixr->midi_tick % PPBAR;
        if (mixr->is_midi_tick && g->m_seq.patterns[g->m_seq.cur_pattern][idx])
        {
            granulator_start(g);
        }
        else if (mixr->is_sixteenth)
            granulator_stop(g);
        seq_tick(&g->m_seq);
    }

    if (g->m_eg1.m_state == OFFF)
        g->sound_generator.active = false;

    osc_update((oscillator *)&g->m_lfo1);
    osc_update((oscillator *)&g->m_lfo2);
    osc_update((oscillator *)&g->m_lfo3);

    if (g->graindur_lfo_on)
    {
        double lfo1_out = lfo_do_oscillate((oscillator *)&g->m_lfo1, NULL);
        double scaley_val =
            scaleybum(-1, 1, g->m_lfo1_min, g->m_lfo1_max, lfo1_out);
        g->grain_duration_ms = scaley_val;
        // if (mixr->start_of_loop && g->lfo1_sync)
        //    g->grain_duration_ms = g->m_lfo1_min;
    }

    if (g->grainps_lfo_on)
    {
        double lfo2_out = lfo_do_oscillate((oscillator *)&g->m_lfo2, NULL);
        double scaley_val =
            scaleybum(-1, 1, g->m_lfo2_min, g->m_lfo2_max, lfo2_out);
        g->grains_per_sec = scaley_val;
        // if (mixr->start_of_loop && g->lfo2_sync)
        //    g->grains_per_sec = g->m_lfo2_min;
    }

    if (g->grainscanfile_lfo_on)
    {
        double lfo3_out = lfo_do_oscillate((oscillator *)&g->m_lfo3, NULL);
        double scaley_val =
            scaleybum(-1, 1, g->m_lfo3_min, g->m_lfo3_max, lfo3_out);
        g->grain_buffer_position = scaley_val;
        if (g->grain_buffer_position >= g->audio_buffer_len)
            g->grain_buffer_position =
                g->grain_buffer_position % g->audio_buffer_len;
        else if (g->grain_buffer_position < 0)
            g->grain_buffer_position =
                g->audio_buffer_len - g->grain_buffer_position;

        // if (mixr->start_of_loop && g->lfo3_sync)
        //    g->grain_buffer_position = g->m_lfo3_min;
    }

    if (g->have_active_buffer) // file buffer or external in
    {
        int spacing = granulator_calculate_grain_spacing(g);
        if (mixr->cur_sample >
            g->last_grain_launched_sample_time + spacing) // new grain time
        {
            g->last_grain_launched_sample_time = mixr->cur_sample;
            g->cur_grain_num = granulator_get_available_grain_num(g);

            int duration = g->grain_duration_ms * 44.1;
            if (g->quasi_grain_fudge != 0)
                duration += rand() % g->quasi_grain_fudge;

            int grain_idx = g->grain_buffer_position;
            if (g->selection_mode == GRAIN_SELECTION_RANDOM)
                grain_idx = rand() % (g->audio_buffer_len - duration);

            if (g->granular_spray > 0)
                grain_idx += rand() % g->granular_spray;

            int attack_time_pct = g->grain_attack_time_pct;
            int release_time_pct = g->grain_release_time_pct;
            sound_grain_init(&g->m_grains[g->cur_grain_num], duration,
                             grain_idx, attack_time_pct, release_time_pct,
                             g->grain_pitch);
            g->num_active_grains++;
            int num_deactivated = granulator_deactivate_other_grains(g);
            g->num_active_grains -= num_deactivated;
        }

        for (int i = 0; i < g->highest_grain_num; i++)
        {
            int grain_idx = sound_grain_generate_idx(&g->m_grains[i]);
            if (grain_idx != -99)
            {
                if (grain_idx < 0)
                    printf("VLIMEY!\n");
                int modified_idx = grain_idx % g->audio_buffer_len;
                val += g->audio_buffer[modified_idx] *
                       sound_grain_env(&g->m_grains[i],
                                       0 /* zero means first grain idx*/);
            }
            grain_idx = sound_grain_gen_doppelganger_idx(&g->m_grains[i]);
            if (grain_idx != -99)
            {
                int modified_idx = grain_idx % g->audio_buffer_len;
                val += g->audio_buffer[modified_idx] *
                       sound_grain_env(&g->m_grains[i], 1 /* 1 means
                       doppelganger grain idx*/);
            }
        }
    }

    val = effector(&g->sound_generator, val);
    val = envelopor(&g->sound_generator, val);

    eg_update(&g->m_eg1);
    double eg_amp = eg_do_envelope(&g->m_eg1, NULL);

    double out_val = val * g->vol * eg_amp;

    return (stereo_val){.left = out_val, .right = out_val};
}

void granulator_status(void *self, wchar_t *status_string)
{
    granulator *g = (granulator *)self;
    swprintf(status_string, MAX_PS_STRING_SZ,
             L"[GRANULATOR] vol:%.2lf source:%s extsource:%d len:%d "
             "quasi_grain_fudge:%d"
             " grain_duration_ms:%d grains_per_sec:%d grain_spray_ms:%d\n"
             "      grain_file_pos:%d grain_pitch:%d selection_mode:%d "
             "active_grains:%d highest_grain_num:%d sequencer_mode:%s\n"
             "      graindur_lfo_on :%s lfo1_type:%d lfo1_amp:%f lfo1_rate:%f"
             " lfo1_min:%f lfo1_max:%f \n"
             "      grainps_lfo_on  :%s lfo2_type:%d lfo2_amp:%f lfo2_rate:%f"
             " lfo2_min:%f lfo2_max:%f \n"
             "      grainscan_lfo_on:%s lfo3_type:%d lfo3_amp:%f lfo3_rate:%f"
             " lfo3_min:%f lfo3_max:%f \n"
             "      eg_amp_attack_ms:%.2f eg_amp_release_ms:%.2f eg_state:%d\n",
             g->vol, g->filename, g->external_source_sg, g->audio_buffer_len,
             g->quasi_grain_fudge, g->grain_duration_ms, g->grains_per_sec,
             g->granular_spray, g->grain_buffer_position, g->grain_pitch,
             g->selection_mode, g->num_active_grains, g->highest_grain_num,
             g->sequencer_mode ? "true" : "false",
             // s_lfo_mode_names[g->m_lfo1.osc.m_waveform],
             g->graindur_lfo_on ? "true" : "false", g->m_lfo1.osc.m_waveform,
             g->m_lfo1.osc.m_amplitude, g->m_lfo1.osc.m_osc_fo, g->m_lfo1_min,
             g->m_lfo1_max, g->grainps_lfo_on ? "true" : "false",
             g->m_lfo2.osc.m_waveform, g->m_lfo2.osc.m_amplitude,
             g->m_lfo2.osc.m_osc_fo, g->m_lfo2_min, g->m_lfo2_max,
             g->grainscanfile_lfo_on ? "true" : "false",
             g->m_lfo3.osc.m_waveform, g->m_lfo3.osc.m_amplitude,
             g->m_lfo3.osc.m_osc_fo, g->m_lfo3_min, g->m_lfo3_max,
             g->m_eg1.m_attack_time_msec, g->m_eg1.m_release_time_msec,
             g->m_eg1.m_state);

    wchar_t seq_status_string[MAX_PS_STRING_SZ];
    memset(seq_status_string, 0, MAX_PS_STRING_SZ);
    seq_status(&g->m_seq, seq_status_string);
    wcscat(status_string, seq_status_string);
    wcscat(status_string, WANSI_COLOR_RESET);
}

void granulator_start(void *self)
{
    granulator *g = (granulator *)self;
    eg_start_eg(&g->m_eg1);
    g->sound_generator.active = true;
}

void granulator_stop(void *self)
{
    granulator *g = (granulator *)self;
    g->started = false;
    eg_release(&g->m_eg1);
    g->sound_generator.active = false;
}

double granulator_getvol(void *self)
{
    granulator *g = (granulator *)self;
    return g->vol;
}

void granulator_setvol(void *self, double v)
{
    granulator *g = (granulator *)self;
    if (v < 0.0 || v > 1.0)
    {
        return;
    }
    g->vol = v;
}

void granulator_del_self(void *self)
{
    // TODO delete file
    granulator *g = (granulator *)self;
    free(g);
}

void granulator_make_active_track(void *self, int track_num)
{
    // NOOP
    (void)self;
    (void)track_num;
}

int granulator_get_num_tracks(void *self)
{
    (void)self;
    return 1;
}

//////////////////////////// grain stuff //////////////////////////
// granulator functions contuine below

void sound_grain_init(sound_grain *g, int dur, int starting_idx, int attack_pct,
                      int release_pct, int pitch)
{
    g->grain_len_samples = dur;
    g->audiobuffer_start_idx = starting_idx;
    g->audiobuffer_cur_pos = starting_idx;
    g->audiobuffer_pitch = pitch;
    g->attack_time_pct = attack_pct;
    g->release_time_pct = release_pct;
    g->active = true;
    g->doppelganger_started =
        false; // start a second grain env, half way through
    g->doppelganger_idx = starting_idx + (dur / 2);
    g->deactivation_pending = false;
}

int sound_grain_generate_idx(sound_grain *g)
{
    if (!g->active)
        return -99;

    double my_idx = g->audiobuffer_cur_pos;

    g->audiobuffer_cur_pos += g->audiobuffer_pitch;
    int end_buffer = g->audiobuffer_start_idx + g->grain_len_samples;
    if (g->audiobuffer_cur_pos >= end_buffer)
    {
        g->audiobuffer_cur_pos -= g->grain_len_samples;
        if (g->deactivation_pending)
            g->active = false;
    }
    else if (g->audiobuffer_cur_pos < g->audiobuffer_start_idx)
    {
        g->audiobuffer_cur_pos = end_buffer - g->audiobuffer_cur_pos;
        if (g->deactivation_pending)
            g->active = false;
    }

    if (g->doppelganger_started)
    { // started by grain_envelope, when we enter
        // release
        g->doppelganger_idx += g->audiobuffer_pitch;
        if (g->doppelganger_idx >= end_buffer)
            g->doppelganger_idx -= g->grain_len_samples;
        else if (g->doppelganger_idx < g->audiobuffer_start_idx)
            g->doppelganger_idx += g->grain_len_samples;
    }

    return my_idx;
}

int sound_grain_gen_doppelganger_idx(sound_grain *g)
{
    if (!g->doppelganger_started)
        return -99;
    else
        return g->doppelganger_idx;
}

double sound_grain_env(sound_grain *g, int idx_num)
{
    int idx = 0;
    if (idx_num == 0)
        idx = g->audiobuffer_cur_pos;
    else if (idx_num == 1)
        idx = g->doppelganger_idx;

    double env_amp = 1.;
    double percent_pos =
        100. / g->grain_len_samples * (idx - g->audiobuffer_start_idx);
    if (percent_pos < g->attack_time_pct)
        env_amp *= percent_pos / g->attack_time_pct;
    else if (percent_pos > (100 - g->release_time_pct))
        env_amp *= (100 - percent_pos) / g->release_time_pct;
    return env_amp;
}

//////////////////////////// end of grain stuff //////////////////////////

void granulator_import_file(granulator *g, char *filename)
{
    strncpy(g->filename, filename, 512);
    audio_buffer_details deetz =
        import_file_contents(&g->audio_buffer, filename);
    g->audio_buffer_len = deetz.buffer_length;
    g->external_source_sg = -1;
    g->have_active_buffer = true;
}

void granulator_set_external_source(granulator *g, int sound_gen_num)
{
    if (mixer_is_valid_soundgen_num(mixr, sound_gen_num))
    {
        g->external_source_sg = sound_gen_num;
        int looplen = mixr->loop_len_in_samples;
        double *buffer = calloc(looplen, sizeof(double));
        if (buffer)
        {
            if (g->audio_buffer)
                free(g->audio_buffer);
            g->audio_buffer = buffer;
            g->audio_buffer_len = looplen;
        }
    }
    g->have_active_buffer = true;
}

int granulator_calculate_grain_spacing(granulator *g)
{
    int looplen_in_seconds = mixr->loop_len_in_samples / (double)SAMPLE_RATE;
    g->num_grains_per_looplen = looplen_in_seconds * g->grains_per_sec;
    if (g->num_grains_per_looplen == 0)
    {
        g->num_grains_per_looplen = 2; // whoops! dn't wanna div by 0 below
    }
    int spacing = mixr->loop_len_in_samples / g->num_grains_per_looplen;
    return spacing;
}

void granulator_set_grain_duration(granulator *g, int dur)
{
    // if (dur < MAX_GRAIN_DURATION) {
    g->grain_duration_ms = dur;
    //} else
    //    printf("Sorry, grain duration must be under %d\n",
    //    MAX_GRAIN_DURATION);
}

void granulator_set_grains_per_sec(granulator *g, int gps)
{
    g->grains_per_sec = gps;
}

void granulator_set_grain_attack_size_pct(granulator *g, int attack_pct)
{
    if (attack_pct < 50)
        g->grain_attack_time_pct = attack_pct;
}

void granulator_set_grain_release_size_pct(granulator *g, int release_pct)
{
    if (release_pct < 50)
        g->grain_release_time_pct = release_pct;
}

void granulator_set_grain_buffer_position(granulator *g, int pos)
{
    if (pos < 0 || pos > 100)
    {
        printf("file position should be a percent\n");
        return;
    }
    g->grain_buffer_position = (double)pos / 100. * g->audio_buffer_len;
}

void granulator_set_granular_spray(granulator *g, int spray_ms)
{
    int spray_samples = spray_ms * 44.1;
    g->granular_spray = spray_samples;
}

void granulator_set_quasi_grain_fudge(granulator *g, int fudgefactor)
{
    g->quasi_grain_fudge = fudgefactor;
}

void granulator_set_grain_pitch(granulator *g, int pitch)
{
    g->grain_pitch = pitch;
}

void granulator_set_selection_mode(granulator *g, unsigned int mode)
{
    if (mode >= GRAIN_NUM_SELECTION_MODES)
    {
        printf("Selection must be < %d\n", GRAIN_NUM_SELECTION_MODES);
        return;
    }
    g->selection_mode = mode;
}

void granulator_set_sequencer_mode(granulator *g, bool b)
{
    if (b != 0 && b != 1)
    {
        printf("not BOOLEY BOOLEY!\n");
        return;
    }
    if (b)
        g->sequencer_gate = 0;
    else
        g->sequencer_gate = 1;
    g->sequencer_mode = b;
}

int granulator_get_available_grain_num(granulator *g)
{
    int idx = 0;
    while (idx < MAX_CONCURRENT_GRAINS)
    {
        if (!g->m_grains[idx].active)
        {
            if (idx > g->highest_grain_num)
                g->highest_grain_num = idx;
            return idx;
        }
        idx++;
    }
    printf("WOW - NO GRAINS TO BE FOUND IN %d attempts\n", idx++);
    return 0;
}

int granulator_deactivate_other_grains(granulator *g)
{
    int num_deactivated = 0;
    for (int i = 0; i < g->highest_grain_num; i++)
    {
        if (i == g->cur_grain_num)
            continue;
        if (g->m_grains[i].active && !g->m_grains[i].deactivation_pending)
        {
            g->m_grains[i].deactivation_pending = true;
            num_deactivated++;
        }
    }
    return num_deactivated;
}

void granulator_set_lfo_amp(granulator *g, int lfonum, double amp)
{
    if (amp >= 0.0 && amp <= 1.0)
    {
        switch (lfonum)
        {
        case (1):
            g->m_lfo1.osc.m_amplitude = amp;
            break;
        case (2):
            g->m_lfo2.osc.m_amplitude = amp;
            break;
        case (3):
            g->m_lfo3.osc.m_amplitude = amp;
            break;
        }
    }
    else
        printf("Amp should be between 0 and 1\n");
}

void granulator_set_lfo_voice(granulator *g, int lfonum, unsigned int voice)
{
    if (voice < MAX_LFO_OSC)
    {
        switch (lfonum)
        {
        case (1):
            g->m_lfo1.osc.m_waveform = voice;
            break;
        case (2):
            g->m_lfo2.osc.m_waveform = voice;
            break;
        case (3):
            g->m_lfo3.osc.m_waveform = voice;
            break;
        }
    }
    else
        printf("Voice ENUM should be < %d\n", MAX_LFO_OSC);
}

void granulator_set_lfo_rate(granulator *g, int lfonum, double rate)
{
    // if (rate >= MIN_LFO_RATE && rate <= MAX_LFO_RATE)
    if (rate >= 0 && rate <= MAX_LFO_RATE)
    {
        switch (lfonum)
        {
        case (1):
            g->m_lfo1.osc.m_osc_fo = rate;
            g->lfo1_sync = false;
            break;
        case (2):
            g->m_lfo2.osc.m_osc_fo = rate;
            g->lfo2_sync = false;
            break;
        case (3):
            g->m_lfo3.osc.m_osc_fo = rate;
            g->lfo3_sync = false;
            break;
        }
    }
    else
        printf("LFO rate should be between %f and %f\n", 0., MAX_LFO_RATE);
}

void granulator_set_lfo_min(granulator *g, int lfonum, double minval)
{
    if (minval < 0)
        return;
    switch (lfonum)
    {
    case (1):
        g->m_lfo1_min = minval;
        break;
    case (2):
        g->m_lfo2_min = minval;
        break;
    case (3):
        g->m_lfo3_min = minval;
        break;
    }
}

void granulator_set_lfo_max(granulator *g, int lfonum, double maxval)
{
    switch (lfonum)
    {
    case (1):
        if (maxval > g->m_lfo1_min)
            g->m_lfo1_max = maxval;
        break;
    case (2):
        if (maxval > g->m_lfo2_min)
            g->m_lfo2_max = maxval;
        break;
    case (3):
        if (maxval > g->m_lfo3_min)
            g->m_lfo3_max = maxval;
        break;
    }
}

void granulator_set_lfo_sync(granulator *g, int lfonum, int numloops)
{

    int looplen_in_samples = 60 / mixr->bpm * SAMPLE_RATE;
    double osc_fo = (double)SAMPLE_RATE / (looplen_in_samples * numloops);
    printf("Setting LFO %d sync to %f\n", lfonum, osc_fo);
    switch (lfonum)
    {
    case (1):
        g->m_lfo1.osc.m_osc_fo = osc_fo;
        g->lfo1_sync = true;
        break;
    case (2):
        g->m_lfo2.osc.m_osc_fo = osc_fo;
        g->lfo2_sync = true;
        break;
    case (3):
        g->m_lfo3.osc.m_osc_fo = osc_fo;
        g->lfo3_sync = true;
        break;
    }
}
