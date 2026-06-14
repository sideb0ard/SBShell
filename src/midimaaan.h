#pragma once

#include <defjams.h>

midi_event new_midi_event(int event_type, int data1, int data2);
int get_midi_note_from_string(char *string);
int get_midi_note_from_mixer_key(unsigned int key, int octave);
