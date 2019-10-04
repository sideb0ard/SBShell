#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "SequenceEngine.h"
#include "bitshift.h"
#include "defjams.h"
#include "euclidean.h"
#include "mixer.h"
#include "utils.h"
#include <drumsampler.h>
#include <drumsynth.h>
#include <pattern_parser.h>
#include <pattern_utils.h>

extern const int key_midi_mapping[NUM_KEYS];
extern const char *key_names[NUM_KEYS];
extern const compat_key_list compat_keys[NUM_KEYS];

const char *s_arp_mode[] = {"UP", "DOWN", "UPDOWN", "RAND"};
const char *s_arp_speed[] = {"32", "24", "16", "12", "8", "6", "4", "3"};

extern mixer *mixr;

SequenceEngine::SequenceEngine()
{
    num_patterns = 1;
    multi_pattern_mode = true;
    cur_pattern_iteration = 1;
    for (int i = 0; i < MAX_NUM_MIDI_LOOPS; i++)
        pattern_multiloop_count[i] = 1;

    sample_rate = 44100;
    sample_rate_counter = 0;

    midi_note_1 = 24; // C0
    midi_note_2 = 32; // G#0
    midi_note_3 = 31; // G0

    octave = 1;

    arp.enable = false;
    arp.direction = UP;
    arp.mode = ARP_UP;
    arp.speed = ARP_16;
    for (int i = 0; i < MAX_NOTES_ARP; i++)
        arp.last_midi_notes[i] = -1;

    sustain_note_ms = 500;
    single_note_mode = true; // opposite is melody mode
    chord_mode = false;
    follow_mixer_chord_changes = true;
    started = false;

    enable_event_mask = false;
    event_mask = 0;
    apply_mask_every_n = 1;
    event_mask_counter = 0;

    pct_play = 95; // %

    sequence_engine_reset_step(this);
    midi_pattern_print(patterns[cur_pattern]);
}

void sequence_engine_set_pattern_to_riff(SequenceEngine *engine)
{
    midi_event *midi_pattern = engine->patterns[engine->cur_pattern];
    for (int i = 0; i < PPBAR; i++)
    {
        midi_event *ev = &midi_pattern[i];
        if (ev->event_type == MIDI_ON || ev->event_type == MIDI_OFF)
        {
            if (i < (PPQN * 12))
                ev->data1 = engine->midi_note_1;
            if (i < (PPQN * 14))
                ev->data1 = engine->midi_note_2;
            else
                ev->data1 = engine->midi_note_3;
        }
    }
}

void sequence_engine_set_pattern_to_current_key(SequenceEngine *engine)
{
    midi_event *midi_pattern = engine->patterns[engine->cur_pattern];

    chord_midi_notes chnotes = {0};
    get_midi_notes_from_chord(mixr->timing_info.chord,
                              mixr->timing_info.chord_type,
                              sequence_engine_get_octave(engine), &chnotes);

    int note2 = chnotes.third;
    if (rand() % 100 > 50)
        note2 = chnotes.seventh;
    if (rand() % 100 > 75)
        note2 = chnotes.fifth;
    if (rand() % 100 > 90)
        note2 = chnotes.seventh;
    for (int i = 0; i < PPBAR; i++)
    {
        midi_event *ev = &midi_pattern[i];
        int midi_note = 0;
        if (ev->event_type == MIDI_ON || ev->event_type == MIDI_OFF)
        {

            if (engine->single_note_mode)
                ev->data1 = chnotes.root;
            else
            {

                midi_note = chnotes.root;
                if (i > (PPBAR / 2))
                {
                    if (rand() % 100 > 70)
                        midi_note = note2;
                }

                if (rand() % 100 > 90)
                    midi_note += 12; // up an octave

                ev->data1 = midi_note;
            }
        }
    }
}

void sequence_engine_set_sample_rate(SequenceEngine *engine, int sample_rate)
{
    // does sample and hold to sample down
    printf("Chh-ch-changing SAMPLE_RATE!: %d\n", sample_rate);
    engine->sample_rate = sample_rate;
    engine->sample_rate_ratio = SAMPLE_RATE / (double)sample_rate;
    engine->sample_rate_counter = 0;
}

void sequence_engine_set_multi_pattern_mode(SequenceEngine *ms,
                                            bool pattern_mode)
{
    ms->multi_pattern_mode = pattern_mode;
    ms->cur_pattern_iteration = ms->pattern_multiloop_count[ms->cur_pattern];
}

void sequence_engine_set_pattern_loop_num(SequenceEngine *self, int pattern_num,
                                          int loop_num)
{
    self->pattern_multiloop_count[pattern_num] = loop_num;
}

int sequence_engine_add_pattern(SequenceEngine *ms)
{
    return ms->num_patterns++;
}

void sequence_engine_dupe_pattern(midi_pattern *from, midi_pattern *to)
{
    for (int i = 0; i < PPBAR; i++)
        (*to)[i] = (*from)[i];
}

void sequence_engine_switch_pattern(SequenceEngine *ms,
                                    unsigned int pattern_num)
{
    if (pattern_num < (unsigned)ms->num_patterns)
        ms->cur_pattern = pattern_num;
}

void sequence_engine_reset_pattern_all(SequenceEngine *ms)
{
    for (int i = 0; i < MAX_NUM_MIDI_LOOPS; i++)
    {
        sequence_engine_reset_pattern(ms, i);
    }
}

void sequence_engine_reset_pattern(SequenceEngine *engine,
                                   unsigned int pattern_num)
{
    if (pattern_num < MAX_NUM_MIDI_LOOPS)
    {
        memset(engine->patterns[pattern_num], 0, sizeof(midi_pattern));
    }
}

// sound generator interface //////////////
void sequence_engine_status(SequenceEngine *engine, wchar_t *status_string)
{
    wchar_t scratch[256] = {0};
    wchar_t patternstr[33] = {0};

    swprintf(
        scratch, 255,
        L"\nsingle_note_mode:%d chord_mode:%d octave:%d sustain_note_ms:%d "
        L"debug:%d\n"
        L"num_patterns:%d midi_note_1:%d midi_note_2:%d midi_note_3:%d "
        L"follow:%d\n"
        L"count_by:%d cur_step:%d incr:%d range:%d fold:%d pct_play:%d\n"
        L"arp:%d [%d,%d,%d] arp_speed:%s arp_mode:%s swing:%d transpose:%d",
        engine->single_note_mode, engine->chord_mode, engine->octave,
        engine->sustain_note_ms, engine->debug, engine->num_patterns,
        engine->midi_note_1, engine->midi_note_2, engine->midi_note_3,
        engine->follow_mixer_chord_changes, engine->count_by, engine->cur_step,
        engine->increment_by, engine->range_len, engine->fold, engine->pct_play,
        engine->arp.enable, engine->arp.last_midi_notes[0],
        engine->arp.last_midi_notes[1], engine->arp.last_midi_notes[2],
        s_arp_speed[engine->arp.speed], s_arp_mode[engine->arp.mode],
        engine->swing_setting, engine->transpose);
    wcscat(status_string, scratch);
    memset(scratch, 0, 256);
    for (int i = 0; i < engine->num_patterns; i++)
    {
        midi_pattern_to_widechar(engine->patterns[i], patternstr);
        swprintf(scratch, 255, L"\n[%d]  %ls  numloops: %d", i, patternstr,
                 engine->pattern_multiloop_count[i]);
        wcscat(status_string, scratch);
    }
    memset(scratch, 0, 256);
    memset(patternstr, 0, 33);
    mask_to_string(engine->event_mask, patternstr);
    swprintf(scratch, 255, L"\n     %ls  mask (%d) mask_every %d", patternstr,
             engine->event_mask, engine->apply_mask_every_n);
    wcscat(status_string, scratch);
    wcscat(status_string, WANSI_COLOR_RESET);
}

void sequence_engine_add_event(SequenceEngine *engine, int pattern_num,
                               int midi_tick, midi_event ev)
{
    midi_event *pattern = engine->patterns[pattern_num];
    midi_pattern_add_event(pattern, midi_tick, ev);
}

void sequence_engine_add_temporal_event(SequenceEngine *engine, int midi_tick,
                                        midi_event ev)
{
    midi_pattern_add_event(engine->temporal_events, midi_tick, ev);
}

void sequence_engine_clear_pattern_ready_for_new_one(SequenceEngine *ms,
                                                     int pattern_num)
{
    clear_midi_pattern(ms->patterns[pattern_num]);
}

void sequence_engine_nudge_pattern(SequenceEngine *ms, int pattern_num,
                                   int sixteenth)
{
    if (sixteenth >= 16)
        sixteenth = sixteenth % 16;

    int sixteenth_of_loop = PPBAR / 16.0;

    midi_event *source_pattern = sequence_engine_get_pattern(ms, pattern_num);

    midi_pattern new_loop;
    for (int i = 0; i < PPBAR; i++)
    {
        if (source_pattern[i].event_type)
        {
            int new_tick = (i + (sixteenth * sixteenth_of_loop)) % PPBAR;
            new_loop[new_tick] = source_pattern[i];
        }
    }

    pattern_change_info change_info = {.clear_previous = true,
                                       .temporary = false};
    sequence_engine_set_pattern(ms, pattern_num, change_info, new_loop);
}

bool sequence_engine_is_valid_pattern(SequenceEngine *engine, int pattern_num)
{
    if (pattern_num >= 0 && pattern_num < MAX_NUM_MIDI_LOOPS)
    {
        if (pattern_num >= engine->num_patterns)
            engine->num_patterns = pattern_num + 1;
        return true;
    }
    return false;
}

void sequence_engine_set_single_note_mode(SequenceEngine *engine, bool b)
{
    engine->single_note_mode = b;
}

void sequence_engine_set_chord_mode(SequenceEngine *engine, bool b)
{
    engine->chord_mode = b;
}

void sequence_engine_set_backup_mode(SequenceEngine *engine, bool b)
{
    if (b)
    {
        sequence_engine_dupe_pattern(
            &engine->patterns[0], &engine->backup_pattern_while_getting_crazy);
        // engine->m_settings_backup_while_getting_crazy = engine->m_settings;
        engine->multi_pattern_mode = false;
        engine->cur_pattern = 0;
    }
    else
    {
        sequence_engine_dupe_pattern(
            &engine->backup_pattern_while_getting_crazy, &engine->patterns[0]);
        // engine->m_settings = engine->m_settings_backup_while_getting_crazy;
        engine->multi_pattern_mode = true;
    }
}

int sequence_engine_get_num_notes(SequenceEngine *ms)
{
    int notecount = 0;
    for (int i = 0; i < ms->num_patterns; i++)
        for (int j = 0; j < PPBAR; j++)
            if (ms->patterns[i][j].event_type == 144)
                notecount++;
    return notecount;
}

int sequence_engine_get_notes_from_pattern(midi_pattern loop,
                                           int return_midi_notes[10])
{
    int idx = 0;
    for (int i = 0; i < PPBAR; i++)
    {
        midi_event ev = loop[i];
        if (ev.event_type == MIDI_ON)
        { // note on
            if (!is_int_member_in_array(ev.data1, return_midi_notes, 10))
            {
                return_midi_notes[idx++] = ev.data1;
                if (idx == 10)
                    break;
                // return idx;
            }
        }
    }
    return idx; // num notes
}

int sequence_engine_get_num_patterns(SequenceEngine *engine)
{
    return engine->num_patterns;
}

void sequence_engine_set_num_patterns(SequenceEngine *engine, int num_patterns)
{
    if (num_patterns > 0 && num_patterns < MAX_NUM_MIDI_LOOPS)
    {
        engine->num_patterns = num_patterns;
        engine->cur_pattern = 0;
    }
}

void sequence_engine_make_active_pattern(SequenceEngine *engine,
                                         int pattern_num)
{
    engine->cur_pattern = pattern_num;
}

void sequence_engine_print_patterns(SequenceEngine *ms)
{
    for (int i = 0; i < ms->num_patterns; i++)
        midi_pattern_print(ms->patterns[i]);
}

void sequence_engine_add_note(SequenceEngine *engine, int pattern_num, int step,
                              int midi_note, int amp, bool keep_note)
{
    int mstep = step * PPSIXTEENTH;
    sequence_engine_add_micro_note(engine, pattern_num, mstep, midi_note, amp,
                                   keep_note);
}

void sequence_engine_add_micro_note(SequenceEngine *engine, int pattern_num,
                                    int mstep, int midi_note, int amp,
                                    bool keep_note)
{
    if (sequence_engine_is_valid_pattern(engine, pattern_num) && mstep < PPBAR)
    {
        midi_event on = new_midi_event(MIDI_ON, midi_note, amp);

        if (!keep_note)
        {
            on.delete_after_use = true;
        }

        sequence_engine_add_event(engine, pattern_num, mstep, on);
    }
    else
    {
        printf("Adding MICRO note - not valid pattern-num(%d) || step no "
               "good(%d)\n",
               pattern_num, mstep);
    }
}

void sequence_engine_rm_note(SequenceEngine *ms, int pattern_num, int step)
{
    int mstep = step * PPSIXTEENTH;
    sequence_engine_rm_micro_note(ms, pattern_num, mstep);
}

void sequence_engine_rm_micro_note(SequenceEngine *ms, int pat_num, int tick)
{
    if (sequence_engine_is_valid_pattern(ms, pat_num) && tick < PPBAR)
    {
        memset(&ms->patterns[ms->cur_pattern][tick], 0, sizeof(midi_event));
    }
    else
    {
        printf("Not a valid pattern num: %d \n", pat_num);
    }
}

void sequence_engine_mv_note(SequenceEngine *ms, int pattern_num, int fromstep,
                             int tostep)
{
    int mfromstep = fromstep * PPSIXTEENTH;
    int mtostep = tostep * PPSIXTEENTH;
    sequence_engine_mv_micro_note(ms, pattern_num, mfromstep, mtostep);
}

void sequence_engine_mv_micro_note(SequenceEngine *ms, int pattern_num,
                                   int fromstep, int tostep)
{
    if (sequence_engine_is_valid_pattern(ms, pattern_num))
    {
        ms->patterns[pattern_num][tostep] = ms->patterns[pattern_num][fromstep];
        memset(&ms->patterns[pattern_num][fromstep], 0, sizeof(midi_event));
    }
}

void sequence_engine_import_midi_from_file(SequenceEngine *engine,
                                           char *filename)
{
    printf("Importing MIDI from %s\n", filename);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Couldn't open yer file!\n");
        return;
    }

    char *item, *last_s;
    char const *sep = "::";
    char line[4096];
    while (fgets(line, sizeof(line), fp))
    {
        printf("%s", line);
        int max_tick = PPBAR * engine->num_patterns;
        int count = 0;
        int tick = 0;
        int status = 0;
        int midi_note = 0;
        int midi_vel = 0;
        for (item = strtok_r(line, sep, &last_s); item;
             item = strtok_r(NULL, sep, &last_s))
        {
            switch (count)
            {
            case 0:
                tick = atoi(item);
                if (tick >= max_tick)
                {
                    printf("TICK OVER!: %d\n", tick);
                    sequence_engine_add_pattern(engine);
                    tick = tick % PPBAR;
                }
                break;
            case 1:
                status = atoi(item);
                break;
            case 2:
                midi_note = atoi(item);
                break;
            case 3:
                midi_vel = atoi(item);
                break;
            }
            count++;
            printf("ITEM! %s\n", item);
        }
        if (count == 4)
        {
            printf("GOtzz %d %d %d %d %d\n", count, tick, status, midi_note,
                   midi_vel);
            midi_event ev = new_midi_event(status, midi_note, midi_vel);
            sequence_engine_add_event(engine, engine->cur_pattern, tick, ev);
        }
    }

    sequence_engine_set_multi_pattern_mode(engine, true);
    fclose(fp);
}

int sequence_engine_change_octave_pattern(SequenceEngine *engine,
                                          int pattern_num, int direction)
{
    // printf("Changing octave of %d - direction: %s\n", pattern_num,
    //       direction == 1 ? "UP" : "DOWN");
    if (sequence_engine_is_valid_pattern(engine, pattern_num))
    {
        for (int i = 0; i < PPBAR; i++)
            if (engine->patterns[pattern_num][i].event_type)
            {
                int new_midi_num = engine->patterns[pattern_num][i].data1;
                if (direction == 1) // up
                    new_midi_num += 12;
                else
                    new_midi_num -= 12;
                if (new_midi_num >= 0 && new_midi_num < 128)
                    engine->patterns[pattern_num][i].data1 = new_midi_num;
            }
    }
    else
        return 1;

    return 0;
}

void sequence_engine_set_sustain_note_ms(SequenceEngine *engine,
                                         int sustain_note_ms)
{
    if (sustain_note_ms > 0)
        engine->sustain_note_ms = sustain_note_ms;
}

void sequence_engine_set_midi_note(SequenceEngine *engine, int midi_note_num,
                                   int note)
{
    switch (midi_note_num)
    {
    case (1):
        engine->midi_note_1 = note;
        break;
    case (2):
        engine->midi_note_2 = note;
        break;
    case (3):
        engine->midi_note_3 = note;
        break;
    }
}

midi_event *sequence_engine_get_pattern(SequenceEngine *engine, int pattern_num)
{
    if (sequence_engine_is_valid_pattern(engine, pattern_num))
        return engine->patterns[pattern_num];
    return NULL;
}

void sequence_engine_set_pattern(SequenceEngine *engine, int pattern_num,
                                 pattern_change_info change_info,
                                 midi_event *pattern)
{
    if (sequence_engine_is_valid_pattern(engine, pattern_num))
    {
        int default_midi_note =
            get_midi_note_from_mixer_key(mixr->timing_info.key, engine->octave);
        if (change_info.clear_previous)
            clear_midi_pattern(engine->patterns[pattern_num]);
        for (int i = 0; i < PPBAR; i++)
        {
            midi_event ev = pattern[i];
            ev.data1 = default_midi_note;
            if (change_info.temporary)
                ev.delete_after_use = true;
            sequence_engine_add_event(engine, pattern_num, i, ev);
        }
        if (engine->swing_setting > 0)
            pattern_apply_swing(engine->patterns[pattern_num],
                                engine->swing_setting);

        sequence_engine_set_pattern_to_current_key(engine);
    }
}

bool sequence_engine_list_presets(unsigned int synthtype)
{
    FILE *presetzzz = NULL;
    switch (synthtype)
    {
    case (MINISYNTH_TYPE):
        presetzzz = fopen(MOOG_PRESET_FILENAME, "r+");
        break;
    case (DXSYNTH_TYPE):
        presetzzz = fopen(DX_PRESET_FILENAME, "r+");
        break;
    }

    if (presetzzz == NULL)
        return false;

    char line[256];
    char setting_key[512];
    char setting_val[512];

    char *tok, *last_tok;
    char const *sep = "::";

    while (fgets(line, sizeof(line), presetzzz))
    {
        for (tok = strtok_r(line, sep, &last_tok); tok;
             tok = strtok_r(NULL, sep, &last_tok))
        {
            sscanf(tok, "%[^=]=%s", setting_key, setting_val);
            if (strcmp(setting_key, "name") == 0)
            {
                printf("%s\n", setting_val);
                break;
            }
        }
    }

    fclose(presetzzz);

    return true;
}

void sequence_engine_set_octave(SequenceEngine *engine, int octave)
{
    if (octave >= -1 && octave < 10)
        engine->octave = octave;
}

int sequence_engine_get_octave(SequenceEngine *engine)
{
    return engine->octave;
}

void sequence_engine_enable_arp(SequenceEngine *engine, bool b)
{
    engine->arp.enable = b;
}

void arp_add_last_note(arpeggiator *arp, int note)
{
    bool found = false;
    for (int i = 0; i < (MAX_NOTES_ARP); i++)
    {
        if (arp->last_midi_notes[i] == note)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        for (int i = 0; i < (MAX_NOTES_ARP - 1); i++)
            arp->last_midi_notes[i] = arp->last_midi_notes[i + 1];
        arp->last_midi_notes[MAX_NOTES_ARP - 1] = note;
    }
}

int arp_next_note(arpeggiator *arp)
{
    int midi_note = -1;
    bool found = false;
    for (int i = 0; i < 3 && !found; i++)
    {
        if (arp->mode == ARP_UP)
        {
            midi_note = arp->last_midi_notes[arp->last_midi_notes_idx++];
            if (arp->last_midi_notes_idx >= MAX_NOTES_ARP)
                arp->last_midi_notes_idx = 0;
        }
        else if (arp->mode == ARP_DOWN)
        {
            midi_note = arp->last_midi_notes[arp->last_midi_notes_idx--];
            if (arp->last_midi_notes_idx <= 0)
                arp->last_midi_notes_idx = MAX_NOTES_ARP - 1;
        }
        else if (arp->mode == ARP_UPDOWN)
        {
            midi_note = arp->last_midi_notes[arp->last_midi_notes_idx];
            if (arp->direction == UP)
            {
                arp->last_midi_notes_idx++;
                if (arp->last_midi_notes_idx >= MAX_NOTES_ARP)
                {
                    arp->last_midi_notes_idx--;
                    arp->direction = DOWN;
                }
            }
            else
            {
                arp->last_midi_notes_idx--;
                if (arp->last_midi_notes_idx <= 0)
                {
                    arp->last_midi_notes_idx++;
                    arp->direction = UP;
                }
            }
        }
        else if (arp->mode == ARP_RAND)
        {
            midi_note = arp->last_midi_notes[rand() % MAX_NOTES_ARP];
        }

        if (midi_note != -1)
            found = true;
    }

    return midi_note;
}

void sequence_engine_set_arp_speed(SequenceEngine *engine, unsigned int speed)
{
    if (speed < ARP_MAX_SPEEDS)
        engine->arp.speed = speed;
}

void sequence_engine_set_arp_mode(SequenceEngine *engine, unsigned int mode)
{
    if (mode < ARP_MAX_MODES)
        engine->arp.mode = mode;
}

bool sequence_engine_do_arp(SequenceEngine *engine, midi_event *ev)
{
    int midi_note = arp_next_note(&engine->arp);
    if (midi_note != -1)
    {
        int idx = mixr->timing_info.midi_tick % PPBAR;
        if (engine->patterns[engine->cur_pattern][idx].event_type != MIDI_ON)
        {
            ev->event_type = MIDI_ON;
            ev->data1 = midi_note;
            ev->data2 = 128;
            return true;
        }
    }
    return false;
}

void sequence_engine_change_octave_midi_notes(SequenceEngine *engine,
                                              unsigned int direction)
{
    if (direction == UP)
    {
        engine->midi_note_1 += 12;
        engine->midi_note_2 += 12;
        engine->midi_note_3 += 12;
    }
    else
    {
        engine->midi_note_1 -= 12;
        engine->midi_note_2 -= 12;
        engine->midi_note_3 -= 12;
    }
}

void sequence_engine_set_event_mask(SequenceEngine *engine, uint16_t mask,
                                    int apply_mask_every_n)
{
    engine->event_mask = mask;
    engine->apply_mask_every_n = apply_mask_every_n;
}

void sequence_engine_set_enable_event_mask(SequenceEngine *engine, bool b)
{
    engine->enable_event_mask = b;
}

void sequence_engine_set_mask_every(SequenceEngine *engine,
                                    int apply_mask_every_n)
{
    engine->apply_mask_every_n = apply_mask_every_n;
}

bool sequence_engine_is_masked(SequenceEngine *engine)
{
    bool is_masked = false;
    if (engine->event_mask_counter % engine->apply_mask_every_n == 0)
    {
        int cur_sixteenth = mixr->timing_info.sixteenth_note_tick % 16;
        int cur_bit = 1 << (15 - cur_sixteenth);
        if (cur_bit & engine->event_mask)
            is_masked = true;
    }
    return is_masked;
}

void sequence_engine_set_swing_setting(SequenceEngine *engine,
                                       int swing_setting)
{
    engine->swing_setting = swing_setting;
}
void sequence_engine_set_follow_mixer_chords(SequenceEngine *engine, bool b)
{
    engine->follow_mixer_chord_changes = b;
}

void sequence_engine_set_transpose(SequenceEngine *engine, int transpose)
{
    if (transpose > -128 && transpose < 128)
        engine->transpose = transpose;
}

void sequence_engine_set_count_by(SequenceEngine *engine, int count_by)
{
    engine->cur_step = mixr->timing_info.sixteenth_note_tick % 16;
    engine->count_by = count_by;
}

void sequence_engine_reset_step(SequenceEngine *engine)
{
    printf("RESET!\n");
    engine->count_by = 1;
    engine->cur_step = -1;
    engine->range_start = 0;
    engine->range_len = 16;
    engine->fold = 0;
}

void sequence_engine_set_increment_by(SequenceEngine *engine, int incr)
{
    engine->increment_by = incr;
}

void sequence_engine_set_range_len(SequenceEngine *engine, int range)
{
    engine->range_len = range;
}

void sequence_engine_set_fold(SequenceEngine *engine, bool b)
{
    engine->fold = b;
    engine->fold_direction = FOLD_FWD;
}

void sequence_engine_set_debug(SequenceEngine *engine, bool b)
{
    engine->debug = b;
}

void sequence_engine_reset(SequenceEngine *engine)
{
    engine->started = false;
    sequence_engine_reset_step(engine);
}

void sequence_engine_pattern_to_half_speed(SequenceEngine *engine, int pat_num)
{
    if (sequence_engine_is_valid_pattern(engine, pat_num))
    {
        midi_event *orig_pattern = sequence_engine_get_pattern(engine, pat_num);

        midi_event midi_pattern_first_half[PPBAR] = {};
        midi_event midi_pattern_second_half[PPBAR] = {};

        for (int i = 0; i < PPBAR / 2; i++)
            if (orig_pattern[i].event_type)
                midi_pattern_first_half[i * 2] = orig_pattern[i];

        for (int i = PPBAR / 2; i < PPBAR; i++)
            if (orig_pattern[i].event_type)
                midi_pattern_second_half[i * 2 - PPBAR / 2] = orig_pattern[i];

        pattern_change_info change_info = {.clear_previous = true,
                                           .temporary = false};
        sequence_engine_set_pattern(engine, 0, change_info,
                                    midi_pattern_first_half);
        sequence_engine_set_pattern(engine, 1, change_info,
                                    midi_pattern_second_half);
    }
}

void sequence_engine_pattern_to_double_speed(SequenceEngine *engine,
                                             int pat_num)
{
    if (sequence_engine_is_valid_pattern(engine, pat_num))
    {
        midi_event *orig_pattern = sequence_engine_get_pattern(engine, pat_num);

        midi_event doubled_pattern[PPBAR] = {};

        for (int i = 0; i < PPBAR; i++)
            if (orig_pattern[i].event_type)
                midi_pattern_add_event(doubled_pattern, i / 2, orig_pattern[i]);

        for (int i = PPBAR / 2; i < PPBAR; i++)
            if (doubled_pattern[i].event_type)
                doubled_pattern[i * 2] = doubled_pattern[i];

        pattern_change_info change_info = {.clear_previous = true,
                                           .temporary = false};
        sequence_engine_set_pattern(engine, 0, change_info, doubled_pattern);
    }
}

void sequence_engine_set_pct_play(SequenceEngine *engine, int pct)
{
    if (pct >= 0 && pct <= 100)
        engine->pct_play = pct;
}