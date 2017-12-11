#pragma once

#include "arpeggiator.h"
#include "dca.h"
#include "envelope_generator.h"
#include "filter.h"
#include "keys.h"
#include "midimaaan.h"
#include "modmatrix.h"
#include "oscillator.h"
#include "sound_generator.h"

#include "dxsynth_voice.h"
#include "synthbase.h"

#define MAX_DX_VOICES 16

static const char DX_PRESET_FILENAME[] = "settings/dxsynthpresets.dat";

enum
{
    DX1,
    DX2,
    DX3,
    DX4,
    DX5,
    DX6,
    DX7,
    DX8,
    MAXDX
};

typedef struct dxsynthsettings
{
    char m_settings_name[256];

    // LFO1     // lfo/hi/def
    double m_lfo1_intensity; // 0/1/0
    double m_lfo1_rate;      // 0.02 / 20 / 0.5
    unsigned int m_lfo1_waveform;
    unsigned int m_lfo1_mod_dest1; // none, AmpMod, Vibrato
    unsigned int m_lfo1_mod_dest2;
    unsigned int m_lfo1_mod_dest3;
    unsigned int m_lfo1_mod_dest4;

    // OP1
    unsigned int m_op1_waveform; // SINE, SAW, TRI, SQ
    double m_op1_ratio;          // 0.1/10/1
    double m_op1_detune_cents;   // -100/100/0
    double m_eg1_attack_ms;      // 0/5000/100
    double m_eg1_decay_ms;
    double m_eg1_sustain_lvl; // 0/1/0.707
    double m_eg1_release_ms;
    double m_op1_output_lvl; // 0/99/75

    // OP2
    unsigned int m_op2_waveform; // SINE, SAW, TRI, SQ
    double m_op2_ratio;
    double m_op2_detune_cents;
    double m_eg2_attack_ms;
    double m_eg2_decay_ms;
    double m_eg2_sustain_lvl;
    double m_eg2_release_ms;
    double m_op2_output_lvl;

    // OP3
    unsigned int m_op3_waveform; // SINE, SAW, TRI, SQ
    double m_op3_ratio;
    double m_op3_detune_cents;
    double m_eg3_attack_ms;
    double m_eg3_decay_ms;
    double m_eg3_sustain_lvl;
    double m_eg3_release_ms;
    double m_op3_output_lvl;

    // OP4
    unsigned int m_op4_waveform; // SINE, SAW, TRI, SQ
    double m_op4_ratio;
    double m_op4_detune_cents;
    double m_eg4_attack_ms;
    double m_eg4_decay_ms;
    double m_eg4_sustain_lvl;
    double m_eg4_release_ms;
    double m_op4_output_lvl;
    double m_op4_feedback; // 0/70/0

    // VOICE
    double m_portamento_time_ms; // 0/5000/0
    double m_volume_db;          // -96/20/0
    int m_pitchbend_range;       // 0/12/1
    unsigned int m_voice_mode;   // DX[1-8];
    bool m_velocity_to_attack_scaling;
    bool m_note_number_to_decay_scaling;
    bool m_reset_to_zero;
    bool m_legato_mode;

} dxsynthsettings;

typedef struct dxsynth
{
    soundgenerator sound_generator;
    synthbase base;

    dxsynth_voice *m_voices[MAX_DX_VOICES];

    // global modmatrix, core is shared by all voices
    modmatrix m_global_modmatrix; // routing structure for sound generation
    global_synth_params m_global_synth_params;

    dxsynthsettings m_settings;
    dxsynthsettings m_settings_backup_while_getting_crazy;

    double vol;
    double m_last_note_frequency;

} dxsynth;

dxsynth *new_dxsynth(void);
void dxsynth_del_self(void *self);

// sound generator interface //////////////
stereo_val dxsynth_gennext(void *self);
void dxsynth_status(void *self, wchar_t *status_string);
void dxsynth_setvol(void *self, double v);
double dxsynth_getvol(void *self);
void dxsynth_sg_start(void *self);
void dxsynth_sg_stop(void *self);
int dxsynth_get_num_tracks(void *self);
void dxsynth_make_active_track(void *self, int tracknum);

////////////////////////////////////

bool dxsynth_prepare_for_play(dxsynth *synth);
void dxsynth_stop(dxsynth *ms);
void dxsynth_update(dxsynth *synth);

void dxsynth_midi_control(dxsynth *self, unsigned int data1,
                          unsigned int data2);

void dxsynth_increment_voice_timestamps(dxsynth *synth);
dxsynth_voice *dxsynth_get_oldest_voice(dxsynth *synth);
dxsynth_voice *dxsynth_get_oldest_voice_with_note(dxsynth *synth,
                                                  unsigned int midi_note);

bool dxsynth_midi_note_on(dxsynth *self, unsigned int midinote,
                          unsigned int velocity);
bool dxsynth_midi_note_off(dxsynth *self, unsigned int midinote,
                           unsigned int velocity, bool all_notes_off);
void dxsynth_midi_mod_wheel(dxsynth *self, unsigned int data1,
                            unsigned int data2);
void dxsynth_midi_pitchbend(dxsynth *self, unsigned int data1,
                            unsigned int data2);
void dxsynth_reset_voices(dxsynth *self);

void dxsynth_rand_settings(dxsynth *ms);

void dxsynth_print_settings(dxsynth *ms);
void dxsynth_print_melodies(dxsynth *ms);
void dxsynth_print_modulation_routings(dxsynth *ms);
void dxsynth_print_lfo1_routing_info(dxsynth *ms, wchar_t *scratch);
void dxsynth_print_lfo2_routing_info(dxsynth *ms, wchar_t *scratch);
void dxsynth_print_eg1_routing_info(dxsynth *ms, wchar_t *scratch);
void dxsynth_print_eg2_routing_info(dxsynth *ms, wchar_t *scratch);

bool dxsynth_save_settings(dxsynth *ms, char *preset_name);
bool dxsynth_load_settings(dxsynth *ms, char *preset_name);
bool dxsynth_list_presets(void);
bool dxsynth_check_if_preset_exists(char *preset_to_find);

// void dxsynth_set_arpeggiate(dxsynth *ms, bool b);
// void dxsynth_set_arpeggiate_latch(dxsynth *ms, bool b);
// void dxsynth_set_arpeggiate_single_note_repeat(dxsynth *ms, bool b);
// void dxsynth_set_arpeggiate_octave_range(dxsynth *ms, int val);
// void dxsynth_set_arpeggiate_mode(dxsynth *ms, unsigned int mode);
// void dxsynth_set_arpeggiate_rate(dxsynth *ms, unsigned int mode);

void dxsynth_set_bitwise(dxsynth *ms, bool b);
void dxsynth_set_bitwise_mode(dxsynth *ms, int mode);

void dxsynth_set_filter_mod(dxsynth *ms, double mod);

void dxsynth_print(dxsynth *ms);

void dxsynth_set_lfo1_intensity(dxsynth *d, double val);
void dxsynth_set_lfo1_rate(dxsynth *d, double val);
void dxsynth_set_lfo1_waveform(dxsynth *d, unsigned int val);
void dxsynth_set_lfo1_mod_dest(dxsynth *d, unsigned int mod_dest,
                               unsigned int dest);

void dxsynth_set_op_waveform(dxsynth *d, unsigned int op, unsigned int val);
void dxsynth_set_op_ratio(dxsynth *d, unsigned int op, double val);
void dxsynth_set_op_detune(dxsynth *d, unsigned int op, double val);
void dxsynth_set_eg_attack_ms(dxsynth *d, unsigned int eg, double val);
void dxsynth_set_eg_decay_ms(dxsynth *d, unsigned int eg, double val);
void dxsynth_set_eg_release_ms(dxsynth *d, unsigned int eg, double val);
void dxsynth_set_eg_sustain_lvl(dxsynth *d, unsigned int eg, double val);
void dxsynth_set_op_output_lvl(dxsynth *d, unsigned int op, double val);
void dxsynth_set_op4_feedback(dxsynth *d, double val);

void dxsynth_set_portamento_time_ms(dxsynth *d, double val);
void dxsynth_set_volume_db(dxsynth *d, double val);
void dxsynth_set_pitchbend_range(dxsynth *d, unsigned int val);
void dxsynth_set_voice_mode(dxsynth *d, unsigned int val);
void dxsynth_set_velocity_to_attack_scaling(dxsynth *d, bool b);
void dxsynth_set_note_number_to_decay_scaling(dxsynth *d, bool b);
void dxsynth_set_reset_to_zero(dxsynth *d, bool b);
void dxsynth_set_legato_mode(dxsynth *d, bool b);