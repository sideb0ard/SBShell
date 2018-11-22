#include <libgen.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defjams.h"
#include "looper.h"
#include "mixer.h"
#include "utils.h"
#include <pattern_utils.h>

extern mixer *mixr;
extern char *s_lfo_mode_names;

static char *s_env_names[] = {"PARABOLIC", "TRAPEZOIDAL", "TUKEY", "GENERATOR"};
static char *s_loop_mode_names[] = {"LOOP", "STATIC", "SMUDGE"};

looper *new_looper(char *filename)
{
    looper *g = (looper *)calloc(1, sizeof(looper));
    g->vol = 0.7;
    g->have_active_buffer = false;

    g->audio_buffer_read_idx = 0;
    g->granular_spray_frames = 441; // 10ms * (44100/1000)
    g->grain_attack_time_pct = 15;
    g->grain_release_time_pct = 15;
    g->quasi_grain_fudge = 220;
    g->selection_mode = GRAIN_SELECTION_STATIC;
    g->envelope_mode = LOOPER_ENV_PARABOLIC;
    g->envelope_taper_ratio = 0.5;
    g->reverse_mode = 0; // bool
    g->external_source_sg = -1;
    g->buffer_is_full = false;
    g->should_start_recording = false;

    g->loop_mode = LOOPER_LOOP_MODE;
    g->loop_len = 1;
    g->scramble_mode = false;
    g->stutter_mode = false;
    g->stutter_idx = 0;

    g->grain_pitch = 1;

    g->density_duration_sync = true;
    g->fill_factor = 3.;
    looper_set_grain_density(g, 30);

    g->sound_generator.gennext = &looper_gennext;
    g->sound_generator.status = &looper_status;
    g->sound_generator.getvol = &looper_getvol;
    g->sound_generator.setvol = &looper_setvol;
    g->sound_generator.start = &looper_start;
    g->sound_generator.stop = &looper_stop;
    g->sound_generator.get_num_patterns = &looper_get_num_patterns;
    g->sound_generator.set_num_patterns = &looper_set_num_patterns;
    g->sound_generator.make_active_track = &looper_make_active_track;
    g->sound_generator.self_destruct = &looper_del_self;
    g->sound_generator.event_notify = &looper_event_notify;
    g->sound_generator.get_pattern = &looper_get_pattern;
    g->sound_generator.set_pattern = &looper_set_pattern;
    g->sound_generator.is_valid_pattern = &looper_is_valid_pattern;
    g->sound_generator.type = LOOPER_TYPE;

    if (strncmp(filename, "none", 4) != 0)
        looper_import_file(g, filename);

    sequence_engine_init(&g->engine, (void *)g, DRUMSYNTH_TYPE);
    g->engine.sustain_note_ms = 500;

    envelope_generator_init(&g->m_eg1); // start/stop env
    g->m_eg1.m_attack_time_msec = 10;
    g->m_eg1.m_release_time_msec = 50;

    g->gate_mode = false;
    looper_start(g);

    return g;
}

bool looper_is_valid_pattern(void *self, int pattern_num)
{
    looper *l = (looper *)self;
    return is_valid_pattern_num(&l->engine, pattern_num);
}

midi_event *looper_get_pattern(void *self, int pattern_num)
{
    sequence_engine *engine = get_sequence_engine(self);
    if (engine)
        return sequence_engine_get_pattern(engine, pattern_num);
    return NULL;
}

void looper_set_pattern(void *self, int pattern_num,
                        pattern_change_info change_info, midi_event *pattern)
{
    sequence_engine *engine = get_sequence_engine(self);
    if (engine)
        sequence_engine_set_pattern(engine, pattern_num, change_info, pattern);
}

void looper_event_notify(void *self, unsigned int event_type)
{
    looper *g = (looper *)self;

    switch (event_type)
    {
    case (TIME_START_OF_LOOP_TICK):

        g->started = true;
        g->step_diff = 0;

        if (g->scramble_pending)
        {
            g->scramble_mode = true;
            g->scramble_pending = false;
        }
        else
            g->scramble_mode = false;

        if (g->stutter_pending)
        {
            g->stutter_mode = true;
            g->stutter_pending = false;
        }
        else
            g->stutter_mode = false;

        if (!g->should_start_recording)
            g->should_start_recording = true;

        break;

    case (TIME_MIDI_TICK):
        if (g->loop_mode == LOOPER_LOOP_MODE)
        {
            int pulses_per_loop = PPBAR * g->loop_len;

            double rel_pos = mixr->timing_info.midi_tick % pulses_per_loop;
            double decimal_percent_of_loop = rel_pos / pulses_per_loop;
            double new_read_idx = decimal_percent_of_loop * g->audio_buffer_len;

            if (g->reverse_mode)
                new_read_idx = (g->audio_buffer_len - 1) - new_read_idx;

            // this ensures new_read_idx is even
            if (g->num_channels == 2)
                new_read_idx -= ((int)new_read_idx & 1);

            if (g->scramble_mode)
            {
                g->audio_buffer_read_idx =
                    new_read_idx + (g->scramble_diff * g->size_of_sixteenth);
            }
            else if (g->stutter_mode)
            {
                int cur_sixteenth = mixr->timing_info.sixteenth_note_tick % 16;
                int rel_pos_within_a_sixteenth =
                    new_read_idx - (cur_sixteenth * g->size_of_sixteenth);
                g->audio_buffer_read_idx =
                    (g->stutter_idx * g->size_of_sixteenth) +
                    rel_pos_within_a_sixteenth;
            }
            else
                g->audio_buffer_read_idx =
                    new_read_idx + (g->step_diff * g->size_of_sixteenth);
        }

        // step sequencer
        int idx = mixr->timing_info.midi_tick % PPBAR;
        if (g->engine.patterns[g->engine.cur_pattern][idx].event_type ==
            MIDI_ON)
        {
            // printf("[%d] ON\n", idx);

            eg_start_eg(&g->m_eg1);

            midi_event *pattern = g->engine.patterns[g->engine.cur_pattern];
            int off_tick = (int)(idx + ((g->m_eg1.m_attack_time_msec +
                                         g->m_eg1.m_decay_time_msec +
                                         g->engine.sustain_note_ms) *
                                        mixr->timing_info.ms_per_midi_tick)) %
                           PPBAR;
            midi_event off_event = new_midi_event(MIDI_OFF, 0, 128);
            midi_pattern_add_event(pattern, off_tick, off_event);
        }
        else if (g->engine.patterns[g->engine.cur_pattern][idx].event_type ==
                 MIDI_OFF)
        {
            // printf("[%d] OFF\n", idx);
            midi_event_clear(&g->engine.patterns[g->engine.cur_pattern][idx]);
            g->m_eg1.m_state = RELEASE;
        }

        break;

    case (TIME_SIXTEENTH_TICK):

        if (g->scramble_mode)
        {
            g->scramble_diff = 0;
            int cur_sixteenth = mixr->timing_info.sixteenth_note_tick % 16;
            if (cur_sixteenth % 2 != 0)
            {
                int randy = rand() % 100;
                if (randy < 25) // repeat the third 16th
                    g->scramble_diff = 3 - cur_sixteenth;
                else if (randy > 25 && randy < 50) // repeat the 4th sixteenth
                    g->scramble_diff = 4 - cur_sixteenth;
                else if (randy > 25 && randy < 50) // repeat the 7th sixteenth
                    g->scramble_diff = 7 - cur_sixteenth;
            }
        }
        if (g->stutter_mode)
        {
            if (rand() % 100 > 75)
                g->stutter_idx++;
            if (g->stutter_idx == 16)
                g->stutter_idx = 0;
        }
        break;
    }
}

stereo_val looper_gennext(void *self)
{
    looper *g = (looper *)self;
    stereo_val val = {0., 0.};

    if (!g->started || !g->sound_generator.active)
        return val;

    if (g->stop_pending && g->m_eg1.m_state == OFFF)
        g->sound_generator.active = false;

    if (g->external_source_sg != -1 && !g->buffer_is_full)
    {
        if (g->should_start_recording)
        {
            if (mixer_is_valid_soundgen_num(mixr, g->external_source_sg))
            {
                g->audio_buffer[g->audio_buffer_write_idx] =
                    mixr->soundgen_cur_val[g->external_source_sg].left;
                g->audio_buffer[g->audio_buffer_write_idx + 1] =
                    mixr->soundgen_cur_val[g->external_source_sg].right;
                g->audio_buffer_write_idx = g->audio_buffer_write_idx + 2;
                if (g->audio_buffer_write_idx >= g->audio_buffer_len)
                {
                    g->audio_buffer_write_idx = 0;
                    g->buffer_is_full = true;
                }
            }
        }
    }

    if (g->have_active_buffer) // file buffer or external in
    {
        // STEP 1 - calculate if we should launch a new grain
        int spacing = looper_calculate_grain_spacing(g);
        if (mixr->timing_info.cur_sample >
            g->last_grain_launched_sample_time + spacing) // new grain time
        {
            g->last_grain_launched_sample_time = mixr->timing_info.cur_sample;
            g->cur_grain_num = looper_get_available_grain_num(g);

            int duration = g->grain_duration_ms * 44.1;
            if (g->quasi_grain_fudge != 0)
                duration += rand() % (int)(g->quasi_grain_fudge * 44.1);

            int grain_idx = g->audio_buffer_read_idx;
            if (g->selection_mode == GRAIN_SELECTION_RANDOM)
                grain_idx = rand() % (g->audio_buffer_len -
                                      (duration * g->num_channels));

            if (g->granular_spray_frames > 0)
                grain_idx += rand() % g->granular_spray_frames;

            int attack_time_pct = g->grain_attack_time_pct;
            int release_time_pct = g->grain_release_time_pct;

            sound_grain_init(&g->m_grains[g->cur_grain_num], duration,
                             grain_idx, attack_time_pct, release_time_pct,
                             g->reverse_mode, g->grain_pitch, g->num_channels);
            g->num_active_grains = looper_count_active_grains(g);
        }

        // STEP 2 - gather vals from all active grains
        for (int i = 0; i < g->highest_grain_num; i++)
        {
            sound_grain *sgr = &g->m_grains[i];
            stereo_val tmp =
                sound_grain_generate(sgr, g->audio_buffer, g->audio_buffer_len);
            double env = sound_grain_env(sgr, g->envelope_mode);

            val.left += tmp.left * env;
            val.right += tmp.right * env;
        }
    }

    val.left = effector(&g->sound_generator, val.left);
    val.right = effector(&g->sound_generator, val.right);

    eg_update(&g->m_eg1);
    double eg_amp = eg_do_envelope(&g->m_eg1, NULL);

    val.left = val.left * g->vol * eg_amp;
    val.right = val.right * g->vol * eg_amp;

    return val;
}

void looper_status(void *self, wchar_t *status_string)
{
    looper *g = (looper *)self;
    char *INSTRUMENT_COLOR = ANSI_COLOR_RESET;
    if (g->sound_generator.active)
        INSTRUMENT_COLOR = ANSI_COLOR_RED;

    swprintf(
        status_string, MAX_STATIC_STRING_SZ,
        WANSI_COLOR_WHITE
        // clang-format off
        "source:%s %s vol:%.2lf pitch:%.2f stereo:%d\n"
        "mode:%s gate_mode:%d idx:%.0f buf_len:%d atk:%d rel:%d\n"
        "len:%.2f scramble:%d stutter:%d step:%d reverse:%d "
        "buffer_is_full:%d\n"
        "extsource:%d\n"
        "grain_dur_ms:" "%s" "%d" "%s "
        "grains_per_sec:" "%s" "%d" "%s "
        "density_dur_sync:%d "
        "quasi_grain_fudge:%d\n"
        "fill_factor:%.2f grain_spray_ms:%.2f selection_mode:%d env_mode:%s\n"

        "[" "%s" "Envelope Generator" "%s" "]\n"
        "eg_attack_ms:%.2f eg_release_ms:%.2f eg_state:%d",
        // clang-format on

        g->filename, INSTRUMENT_COLOR, g->vol, g->grain_pitch,
        g->num_channels > 1 ? 1 : 0, s_loop_mode_names[g->loop_mode],
        g->gate_mode, g->audio_buffer_read_idx, g->audio_buffer_len,
        g->grain_attack_time_pct, g->grain_release_time_pct, g->loop_len,
        g->scramble_mode, g->stutter_mode, g->step_mode, g->reverse_mode,
        g->buffer_is_full, g->external_source_sg,

        ANSI_COLOR_WHITE, g->grain_duration_ms, INSTRUMENT_COLOR,
        ANSI_COLOR_WHITE, g->grains_per_sec, INSTRUMENT_COLOR,
        g->density_duration_sync, g->quasi_grain_fudge, g->fill_factor,
        g->granular_spray_frames / 44.1, g->selection_mode,
        s_env_names[g->envelope_mode],

        ANSI_COLOR_WHITE, INSTRUMENT_COLOR, g->m_eg1.m_attack_time_msec,
        g->m_eg1.m_release_time_msec, g->m_eg1.m_state);

    wchar_t local_status_string[MAX_STATIC_STRING_SZ] = {};
    sequence_engine_status(&g->engine, local_status_string);
    wcscat(status_string, local_status_string);

    wcscat(status_string, WANSI_COLOR_RESET);
}

void looper_start(void *self)
{
    looper *l = (looper *)self;
    eg_start_eg(&l->m_eg1);
    l->sound_generator.active = true;
    l->stop_pending = false;
}

void looper_stop(void *self)
{
    looper *l = (looper *)self;
    eg_release(&l->m_eg1);
    l->stop_pending = true;
}

double looper_getvol(void *self)
{
    looper *g = (looper *)self;
    return g->vol;
}

void looper_setvol(void *self, double v)
{
    looper *g = (looper *)self;
    if (v < 0.0 || v > 1.0)
    {
        return;
    }
    g->vol = v;
}

void looper_del_self(void *self)
{
    // TODO delete file
    looper *g = (looper *)self;
    free(g);
}

void looper_make_active_track(void *self, int track_num)
{
    // NOOP
    (void)self;
    (void)track_num;
}

int looper_get_num_patterns(void *self)
{
    (void)self;
    return 1;
}

void looper_set_num_patterns(void *self, int num_patterns)
{
    (void)self;
    (void)num_patterns;
}

//////////////////////////// grain stuff //////////////////////////
// looper functions contuine below

void sound_grain_init(sound_grain *g, int dur, int starting_idx, int attack_pct,
                      int release_pct, bool reverse_mode, double pitch,
                      int num_channels)
{
    g->audiobuffer_start_idx = starting_idx;
    g->grain_len_frames = dur;
    g->grain_counter_frames = 0;
    g->attack_time_pct = attack_pct;
    g->release_time_pct = release_pct;
    g->audiobuffer_num_channels = num_channels;

    g->reverse_mode = reverse_mode;
    if (reverse_mode)
    {
        g->audiobuffer_cur_pos = starting_idx + (dur * num_channels) - 1;
        g->incr = -1.0 * pitch;
    }
    else
    {
        g->audiobuffer_cur_pos = starting_idx;
        g->incr = pitch;
    }

    g->attack_time_samples = dur / 100. * attack_pct;
    g->release_time_samples = dur / 100. * release_pct;

    g->amp = 0;
    double rdur = 1.0 / dur;
    double rdur2 = rdur * rdur;
    g->slope = 4.0 * 1.0 * (rdur - rdur2);
    g->curve = -8.0 * 1.0 * rdur2;

    double loop_len_ms = 1000. * dur / SAMPLE_RATE;
    double attack_time_ms = loop_len_ms / 100. * attack_pct;
    double release_time_ms = loop_len_ms / 100. * release_pct;
    // printf("ATTACKMS: %f RELEASEMS: %f\n", attack_time_ms, release_time_ms);

    envelope_generator_init(&g->eg);
    eg_set_attack_time_msec(&g->eg, attack_time_ms);
    eg_set_decay_time_msec(&g->eg, 0);
    eg_set_release_time_msec(&g->eg, release_time_ms);
    eg_start_eg(&g->eg);

    g->active = true;
}

static inline void sound_grain_check_idx(int *index, int buffer_len)
{
    while (*index < 0.0)
        *index += buffer_len;
    while (*index >= buffer_len)
        *index -= buffer_len;
}

stereo_val sound_grain_generate(sound_grain *g, double *audio_buffer,
                                int audio_buffer_len)
{
    stereo_val out = {0., 0.};
    if (!g->active)
        return out;

    int num_channels = g->audiobuffer_num_channels;

    int read_idx = (int)g->audiobuffer_cur_pos;
    double frac = g->audiobuffer_cur_pos - read_idx;
    sound_grain_check_idx(&read_idx, audio_buffer_len);

    if (num_channels == 1)
    {
        int read_next_idx = read_idx + 1;
        sound_grain_check_idx(&read_next_idx, audio_buffer_len);
        out.left = lin_terp(0, 1, audio_buffer[read_idx],
                            audio_buffer[read_next_idx], frac);
        out.right = out.left;
    }
    else if (num_channels == 2)
    {
        int read_next_idx = read_idx + 2;
        sound_grain_check_idx(&read_next_idx, audio_buffer_len);
        out.left = lin_terp(0, 1, audio_buffer[read_idx],
                            audio_buffer[read_next_idx], frac);

        int read_idx_right = read_idx + 1;
        sound_grain_check_idx(&read_idx_right, audio_buffer_len);
        int read_next_idx_right = read_idx_right + 2;
        sound_grain_check_idx(&read_next_idx_right, audio_buffer_len);
        out.right = lin_terp(0, 1, audio_buffer[read_idx_right],
                             audio_buffer[read_next_idx_right], frac);
    }

    g->audiobuffer_cur_pos += (g->incr * num_channels);

    g->grain_counter_frames++;
    if (g->grain_counter_frames > g->grain_len_frames)
    {
        g->active = false;
    }

    return out;
}

double sound_grain_env(sound_grain *g, unsigned int envelope_mode)
{
    double amp = 1;
    double percent_pos =
        (float)g->grain_counter_frames / g->grain_len_frames * 100;

    switch (envelope_mode)
    {
    case (LOOPER_ENV_PARABOLIC):
        g->amp = g->amp + g->slope;
        g->slope = g->slope + g->curve;
        amp = g->amp;
        break;
    case (LOOPER_ENV_TRAPEZOIDAL):
        if (percent_pos < g->attack_time_pct)
            amp *= (percent_pos / g->attack_time_pct);
        else if (percent_pos > (100 - g->release_time_pct))
            amp *= (100 - percent_pos) / g->release_time_pct;
        break;
    case (LOOPER_ENV_TUKEY_WINDOW):
        if (percent_pos < g->attack_time_pct)
        {
            amp = (1.0 + cos(M_PI + (M_PI * (g->grain_counter_frames /
                                             g->attack_time_samples)) *
                                        (1.0 / 2.0)));
        }
        else if (percent_pos > (100 - g->release_time_pct))
        {
            amp = (1.0 + cos(M_PI * (g->grain_counter_frames /
                                     g->release_time_samples)) *
                             (1.0 / 2.0));
        }
        break;
    case (LOOPER_ENV_GENERATOR):
        amp = eg_do_envelope(&g->eg, NULL);
        // printf("AMP is %f\n", amp);
        if (percent_pos > (100 - g->release_time_pct))
        {
            eg_note_off(&g->eg);
        }
        break;
    }

    return amp;
}

//////////////////////////// end of grain stuff //////////////////////////

void looper_import_file(looper *g, char *filename)
{
    strncpy(g->filename, filename, 512);
    audio_buffer_details deetz =
        import_file_contents(&g->audio_buffer, filename);
    g->audio_buffer_len = deetz.buffer_length;
    g->num_channels = deetz.num_channels;
    g->external_source_sg = -1;
    g->have_active_buffer = true;
    g->size_of_sixteenth = g->audio_buffer_len / 16;
}

void looper_set_external_source(looper *g, int sound_gen_num)
{
    if (mixer_is_valid_soundgen_num(mixr, sound_gen_num))
    {
        g->external_source_sg = sound_gen_num;
        int looplen = mixr->timing_info.loop_len_in_frames * 2; // stereo
        double *buffer = calloc(looplen, sizeof(double));
        if (buffer)
        {
            if (g->audio_buffer)
            {
                free(g->audio_buffer);
                g->buffer_is_full = false;
            }

            g->audio_buffer = buffer;
            g->audio_buffer_len = looplen;
            g->num_channels = 2;
            g->have_active_buffer = true;
            g->size_of_sixteenth = g->audio_buffer_len / 16;
            g->should_start_recording = false; // reset
        }
    }
}

int looper_calculate_grain_spacing(looper *g)
{
    int looplen_in_seconds =
        mixr->timing_info.loop_len_in_frames / (double)SAMPLE_RATE;
    g->num_grains_per_looplen = looplen_in_seconds * g->grains_per_sec;
    if (g->num_grains_per_looplen == 0)
    {
        g->num_grains_per_looplen = 2; // whoops! dn't wanna div by 0 below
    }
    int spacing =
        mixr->timing_info.loop_len_in_frames / g->num_grains_per_looplen;
    return spacing;
}

void looper_set_grain_duration(looper *l, int dur)
{
    l->grain_duration_ms = dur;
    if (l->density_duration_sync)
        l->grains_per_sec = 1000. / (l->grain_duration_ms / l->fill_factor);
}

void looper_set_grain_density(looper *l, int gps)
{
    l->grains_per_sec = gps;
    if (l->density_duration_sync)
        l->grain_duration_ms = 1000. / l->grains_per_sec * l->fill_factor;
}

void looper_set_grain_attack_size_pct(looper *g, int attack_pct)
{
    if (attack_pct < 50)
        g->grain_attack_time_pct = attack_pct;
}

void looper_set_grain_release_size_pct(looper *g, int release_pct)
{
    if (release_pct < 50)
        g->grain_release_time_pct = release_pct;
}

void looper_set_audio_buffer_read_idx(looper *g, int pos)
{
    if (pos < 0 || pos >= g->audio_buffer_len)
    {
        return;
    }
    g->audio_buffer_read_idx = pos;
}

void looper_set_granular_spray(looper *g, int spray_ms)
{
    int spray_frames = spray_ms * 44.1;
    g->granular_spray_frames = spray_frames;
}

void looper_set_quasi_grain_fudge(looper *g, int fudgefactor)
{
    g->quasi_grain_fudge = fudgefactor;
}

void looper_set_grain_pitch(looper *g, double pitch) { g->grain_pitch = pitch; }

void looper_set_selection_mode(looper *g, unsigned int mode)
{
    if (mode >= GRAIN_NUM_SELECTION_MODES)
    {
        printf("Selection must be < %d\n", GRAIN_NUM_SELECTION_MODES);
        return;
    }
    g->selection_mode = mode;
}

void looper_set_envelope_mode(looper *g, unsigned int mode)
{
    if (mode >= LOOPER_ENV_NUM)
    {
        printf("Selection must be < %d\n", LOOPER_ENV_NUM);
        return;
    }
    g->envelope_mode = mode;
}

void looper_set_reverse_mode(looper *g, bool b) { g->reverse_mode = b; }
void looper_set_loop_mode(looper *g, unsigned int m)
{
    g->loop_mode = m;
    g->selection_mode = GRAIN_SELECTION_STATIC;
    if (m == LOOPER_SMUDGE_MODE)
    {
        g->quasi_grain_fudge = 220;
        g->granular_spray_frames = 441; // 10ms * (44100/1000)
    }
    else
    {
        g->quasi_grain_fudge = 0;
        g->granular_spray_frames = 0;
    }
}
void looper_set_scramble_pending(looper *g) { g->scramble_pending = true; }

void looper_set_stutter_pending(looper *g) { g->stutter_pending = true; }

void looper_set_step_mode(looper *g, bool b) { g->step_mode = b; }

void looper_set_loop_len(looper *g, double bars)
{
    if (bars != 0)
        g->loop_len = bars;
}

int looper_get_available_grain_num(looper *g)
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
    printf("WOW - NO GRAINS TO BE FOUND IN %d attempts\n", idx);
    return 0;
}

int looper_count_active_grains(looper *g)
{
    int active = 0;
    for (int i = 0; i < g->highest_grain_num; i++)
        if (g->m_grains[i].active)
            active++;

    return active;
}

void looper_set_fill_factor(looper *l, double fill_factor)
{
    if (fill_factor >= 0. && fill_factor <= 10.)
        l->fill_factor = fill_factor;
}

void looper_set_density_duration_sync(looper *l, bool b)
{
    l->density_duration_sync = b;
}

void looper_dump_buffer(looper *l)
{
    printf("Buffer is full:%d\n", l->buffer_is_full);
    printf("Buffer Len:%d\n", l->audio_buffer_len);
    printf("Rdx Idx:%f Write Idx:%d\n", l->audio_buffer_read_idx,
           l->audio_buffer_write_idx);
    printf("Num active grains:%d\n", l->num_active_grains);
    // for (int i = 0; i < l->audio_buffer_len; i+=2)
    //    printf("Left:%f Right:%f\n", l->audio_buffer[i],
    //    l->audio_buffer[i+1]);
    for (int i = 0; i < l->highest_grain_num; i++)
    {
        sound_grain *g = &l->m_grains[i];
        printf("Grain:%d len:%d, buf_num:%d start_idx:%d cur_pos:%f incr:%f "
               "active:%d\n",
               i, g->grain_len_frames, g->audiobuffer_num,
               g->audiobuffer_start_idx, g->audiobuffer_cur_pos, g->incr,
               g->active);
    }
}
void looper_set_gate_mode(looper *g, bool b) { g->gate_mode = b; }

void looper_set_grain_env_attack_pct(looper *l, int percent)
{
    if (percent > 0 && percent < 100)
        l->grain_attack_time_pct = percent;
}
void looper_set_grain_env_release_pct(looper *l, int percent)
{
    if (percent > 0 && percent < 100)
        l->grain_release_time_pct = percent;
}
