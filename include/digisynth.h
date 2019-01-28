#pragma once

#include <stdbool.h>
#include <wchar.h>

#include "digisynth_voice.h"
#include "sequence_engine.h"
#include "sound_generator.h"

typedef struct digisynth
{
    sound_generator sg;
    sequence_engine engine;

    char audiofile[1024];
    digisynth_voice m_voices[MAX_VOICES];

    double m_last_note_frequency;

} digisynth;

digisynth *new_digisynth(char *filename);
void digisynth_load_wav(digisynth *ds, char *filename);

void digisynth_update(digisynth *ds);

// sound generator interface //////////////
stereo_val digisynth_gennext(void *self);
void digisynth_status(void *self, wchar_t *status_string);
void digisynth_stop(digisynth *d);
void digisynth_sg_start(void *self);
void digisynth_sg_stop(void *self);
void digisynth_del_self(void *self);

////////////////////////////////////

bool digisynth_midi_note_on(digisynth *self, unsigned int midinote,
                            unsigned int velocity);
bool digisynth_midi_note_off(digisynth *self, unsigned int midinote,
                             unsigned int velocity, bool all_notes_off);
// void minisynth_toggle_delay_mode(minisynth *ms);
//
// void minisynth_print_settings(minisynth *ms);
// bool minisynth_save_settings(minisynth *ms, char *preset_name);
// bool minisynth_load_settings(minisynth *ms, char *preset_name);
////bool minisynth_list_presets(void);
////bool minisynth_check_if_preset_exists(char *preset_to_find);
//
// void minisynth_set_vol(minisynth *ms, double val);
// void minisynth_set_reset_to_zero(minisynth *ms, unsigned int val);
