#pragma once

#include <stdbool.h>
#include <wchar.h>

#include "arpeggiator.h"
#include "dca.h"
#include "envelope_generator.h"
#include "filter.h"
#include "keys.h"
#include "midimaaan.h"
#include "modmatrix.h"
#include "oscillator.h"
#include "sound_generator.h"

#include "minisynth_voice.h"
#include "synthbase.h"

static const char PRESET_FILENAME[] = "settings/synthpresets.dat";

typedef struct synthsettings
{
    char m_settings_name[256];

    unsigned int m_voice_mode;
    bool m_monophonic;

    unsigned int m_lfo1_waveform;
    unsigned int m_lfo1_dest;
    unsigned int m_lfo1_mode;
    double m_lfo1_rate;
    double m_lfo1_amplitude;

    // LFO1 -> OSC FO
    double m_lfo1_osc_pitch_intensity;
    bool m_lfo1_osc_pitch_enabled;

    // LFO1 -> FILTER
    double m_lfo1_filter_fc_intensity;
    bool m_lfo1_filter_fc_enabled;

    // LFO1 -> DCA
    double m_lfo1_amp_intensity;
    bool m_lfo1_amp_enabled;
    double m_lfo1_pan_intensity;
    bool m_lfo1_pan_enabled;

    // LFO1 -> Pulse Width
    double m_lfo1_pulsewidth_intensity;
    bool m_lfo1_pulsewidth_enabled;

    unsigned int m_lfo2_waveform;
    unsigned int m_lfo2_dest;
    unsigned int m_lfo2_mode;
    double m_lfo2_rate;
    double m_lfo2_amplitude;

    // LFO2 -> OSC FO
    double m_lfo2_osc_pitch_intensity;
    bool m_lfo2_osc_pitch_enabled;

    // LFO2 -> FILTER
    double m_lfo2_filter_fc_intensity;
    bool m_lfo2_filter_fc_enabled;

    // LFO2 -> DCA
    double m_lfo2_amp_intensity;
    bool m_lfo2_amp_enabled;
    double m_lfo2_pan_intensity;
    bool m_lfo2_pan_enabled;

    // LFO2 -> Pulse Width
    double m_lfo2_pulsewidth_intensity;
    bool m_lfo2_pulsewidth_enabled;

    // EG1
    double m_attack_time_msec;
    double m_decay_time_msec;
    double m_release_time_msec;
    double m_sustain_level;

    double m_volume_db;
    double m_fc_control;
    double m_q_control;

    double m_detune_cents;
    double m_pulse_width_pct;
    double m_sub_osc_db;
    double m_noise_osc_db;

    // EG1 -> OSC
    double m_eg1_osc_intensity;
    bool m_eg1_osc_enabled;

    // EG1 -> FILTER
    double m_eg1_filter_intensity;
    bool m_eg1_filter_enabled;

    // EG1 -> DCA
    double m_eg1_dca_intensity;
    bool m_eg1_dca_enabled;

    double m_filter_keytrack_intensity;

    int m_octave;
    int m_pitchbend_range;

    unsigned int m_legato_mode;
    unsigned int m_reset_to_zero;
    unsigned int m_filter_keytrack;
    unsigned int m_filter_type;
    double m_filter_saturation;

    unsigned int m_nlp;
    unsigned int m_velocity_to_attack_scaling;
    unsigned int m_note_number_to_decay_scaling;
    double m_portamento_time_msec;

    unsigned int m_sustain_override;
    double m_sustain_time_ms;
    double m_sustain_time_sixteenth;

    bool m_bytebeat_active;
} synthsettings;

typedef struct minisynth
{
    soundgenerator sound_generator;
    synthbase base;

    minisynth_voice *m_voices[MAX_VOICES];

    // global modmatrix, core is shared by all voices
    modmatrix m_ms_modmatrix; // routing structure for sound generation
    global_synth_params m_global_synth_params;

    double m_last_note_frequency;

    unsigned int m_midi_rx_channel;

    synthsettings m_settings;
    synthsettings m_settings_backup_while_getting_crazy;

    int m_last_midi_notes[MAX_VOICES];
    arpeggiator m_arp;

    // bytebeat bytr;
    int m_bytebeat_counter;

} minisynth;

minisynth *new_minisynth(void);

// sound generator interface //////////////
stereo_val minisynth_gennext(void *self);
void minisynth_status(void *self, wchar_t *status_string);
void minisynth_setvol(void *self, double v);
double minisynth_getvol(void *self);
void minisynth_sg_start(void *self);
void minisynth_sg_stop(void *self);
int minisynth_get_num_tracks(void *self);
void minisynth_make_active_track(void *self, int tracknum);

////////////////////////////////////

bool minisynth_prepare_for_play(minisynth *synth);
void minisynth_stop(minisynth *ms);
void minisynth_update(minisynth *synth);

void minisynth_midi_control(minisynth *self, unsigned int data1,
                            unsigned int data2);

void minisynth_increment_voice_timestamps(minisynth *synth);
minisynth_voice *minisynth_get_oldest_voice(minisynth *synth);
minisynth_voice *minisynth_get_oldest_voice_with_note(minisynth *synth,
                                                      unsigned int midi_note);

// void minisynth_handle_midi_note(minisynth *ms, int note, int velocity,
//                                bool update_last_midi);
bool minisynth_midi_note_on(minisynth *self, unsigned int midinote,
                            unsigned int velocity);
bool minisynth_midi_note_off(minisynth *self, unsigned int midinote,
                             unsigned int velocity, bool all_notes_off);
void minisynth_midi_mod_wheel(minisynth *self, unsigned int data1,
                              unsigned int data2);
void minisynth_midi_pitchbend(minisynth *self, unsigned int data1,
                              unsigned int data2);
void minisynth_reset_voices(minisynth *self);

void minisynth_rand_settings(minisynth *ms);

void minisynth_print_settings(minisynth *ms);
void minisynth_print_melodies(minisynth *ms);
void minisynth_print_modulation_routings(minisynth *ms);
void minisynth_print_lfo1_routing_info(minisynth *ms, wchar_t *scratch);
void minisynth_print_lfo2_routing_info(minisynth *ms, wchar_t *scratch);
void minisynth_print_eg1_routing_info(minisynth *ms, wchar_t *scratch);
void minisynth_print_eg2_routing_info(minisynth *ms, wchar_t *scratch);

bool minisynth_save_settings(minisynth *ms, char *preset_name);
bool minisynth_load_settings(minisynth *ms, char *preset_name);
bool minisynth_list_presets(void);
bool minisynth_check_if_preset_exists(char *preset_to_find);

void minisynth_set_arpeggiate(minisynth *ms, bool b);
void minisynth_set_arpeggiate_latch(minisynth *ms, bool b);
void minisynth_set_arpeggiate_single_note_repeat(minisynth *ms, bool b);
void minisynth_set_arpeggiate_octave_range(minisynth *ms, int val);
void minisynth_set_arpeggiate_mode(minisynth *ms, unsigned int mode);
void minisynth_set_arpeggiate_rate(minisynth *ms, unsigned int mode);

void minisynth_set_filter_mod(minisynth *ms, double mod);
void minisynth_del_self(void *self);

void minisynth_print(minisynth *ms);

void minisynth_set_attack_time_ms(minisynth *ms, double val);
void minisynth_set_decay_time_ms(minisynth *ms, double val);
void minisynth_set_release_time_ms(minisynth *ms, double val);
void minisynth_set_detune(minisynth *ms, double val);
void minisynth_set_eg1_dca_int(minisynth *ms, double val);
void minisynth_set_eg1_dca_enable(minisynth *ms, int val);
void minisynth_set_eg1_filter_int(minisynth *ms, double val);
void minisynth_set_eg1_filter_enable(minisynth *ms, int val);
void minisynth_set_eg1_osc_int(minisynth *ms, double val);
void minisynth_set_eg1_osc_enable(minisynth *ms, int val);
void minisynth_set_filter_fc(minisynth *ms, double val);
void minisynth_set_filter_fq(minisynth *ms, double val);
void minisynth_set_filter_type(minisynth *ms, unsigned int val);
void minisynth_set_filter_saturation(minisynth *ms, double val);
void minisynth_set_filter_nlp(minisynth *ms, unsigned int val);
void minisynth_set_keytrack_int(minisynth *ms, double val);
void minisynth_set_keytrack(minisynth *ms, unsigned int val);
void minisynth_set_legato_mode(minisynth *ms, unsigned int val);
// LFO
void minisynth_set_lfo_amp_enable(minisynth *ms, int lfo_num, int val);
void minisynth_set_lfo_amp_int(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_amp(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_filter_enable(minisynth *ms, int lfo_num, int val);
void minisynth_set_lfo_filter_fc_int(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_rate(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_pan_enable(minisynth *ms, int lfo_num, int val);
void minisynth_set_lfo_pan_int(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_osc_enable(minisynth *ms, int lfo_num, int val);
void minisynth_set_lfo_osc_int(minisynth *ms, int lfo_num, double val);
void minisynth_set_lfo_wave(minisynth *ms, int lfo_num, unsigned int val);
void minisynth_set_lfo_mode(minisynth *ms, int lfo_num, unsigned int val);

void minisynth_set_note_to_decay_scaling(minisynth *ms, unsigned int val);
void minisynth_set_noise_osc_db(minisynth *ms, double val);
void minisynth_set_octave(minisynth *ms, int val);
void minisynth_set_pitchbend_range(minisynth *ms, int val);
void minisynth_set_portamento_time_ms(minisynth *ms, double val);
void minisynth_set_pulsewidth_pct(minisynth *ms, double val);
void minisynth_set_sub_osc_db(minisynth *ms, double val);
void minisynth_set_sustain(minisynth *ms, double val);
void minisynth_set_sustain_time_ms(minisynth *ms, double val);
void minisynth_set_sustain_time_sixteenth(minisynth *ms, double val);
void minisynth_set_sustain_override(minisynth *ms, bool b);
void minisynth_set_velocity_to_attack_scaling(minisynth *ms, unsigned int val);
void minisynth_set_voice_mode(minisynth *ms, unsigned int val);
void minisynth_set_vol(minisynth *ms, double val);
void minisynth_set_reset_to_zero(minisynth *ms, unsigned int val);
void minisynth_set_monophonic(minisynth *ms, bool b);
void minisynth_set_bytebeat(minisynth *ms, bool b);
void minisynth_add_last_note(minisynth *ms, unsigned int val);