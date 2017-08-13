#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audioutils.h"

void pa_setup(void)
{
    // PA start me up!
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("Errrrr! couldn't initialize Portaudio: %s\n",
               Pa_GetErrorText(err));
        exit(-1);
    }
}

void pa_teardown(void)
{
    //  time to go home!
    PaError err;
    err = Pa_Terminate();
    if (err != paNoError) {
        printf("Errrrr while terminating Portaudio: %s\n",
               Pa_GetErrorText(err));
        exit(-1);
    }
    exit(0);
}

chord_midi_notes get_midi_notes_from_char_chord(const char *chord)
{
    chord_midi_notes chnotes = {0, 0, 0};
    printf("Looking for midi nums for %s\n", chord);

    if (strncmp(chord, "C_MAJOR", 7) == 0) {
        chnotes.root = 12;  // C
        chnotes.third = 16; // E
        chnotes.fifth = 19; // G
    }
    else if (strncmp(chord, "D_MINOR", 7) == 0) {
        chnotes.root = 14;  // D
        chnotes.third = 17; // F
        chnotes.fifth = 21; // A
    }
    else if (strncmp(chord, "D_MAJOR", 7) == 0) {
        chnotes.root = 14;  // D
        chnotes.third = 18; // Gm
        chnotes.fifth = 21; // B
    }
    else if (strncmp(chord, "E_MINOR", 7) == 0) {
        chnotes.root = 16;  // E
        chnotes.third = 19; // G
        chnotes.fifth = 23; // B
    }
    else if (strncmp(chord, "E_MAJOR", 7) == 0) {
        chnotes.root = 16;  // E
        chnotes.third = 20; // Am
        chnotes.fifth = 23; // B
    }
    else if (strncmp(chord, "F_MAJOR", 7) == 0) {
        chnotes.root = 17;  // F
        chnotes.third = 21; // A
        chnotes.fifth = 24; // C
    }
    else if (strncmp(chord, "G_MINOR", 7) == 0) {
        chnotes.root = 19;  // G
        chnotes.third = 22; // Bm
        chnotes.fifth = 26; // D
    }
    else if (strncmp(chord, "G_MAJOR", 7) == 0) {
        chnotes.root = 19;  // G
        chnotes.third = 23; // B
        chnotes.fifth = 26; // D
    }
    else if (strncmp(chord, "A_MINOR", 7) == 0) {
        chnotes.root = 12;  // C
        chnotes.third = 16; // E
        chnotes.fifth = 21; //  A
    }
    else if (strncmp(chord, "A_MAJOR", 7) == 0) {
        chnotes.root = 21;  // A
        chnotes.third = 25; // C#
        chnotes.fifth = 28; // E
    }
    else if (strncmp(chord, "B_MINOR", 7) == 0) {
        chnotes.root = 11;  // B
        chnotes.third = 14; // D
        chnotes.fifth = 18; // Gm
    }
    else if (strncmp(chord, "B_MAJOR", 7) == 0) {
        chnotes.root = 11;  // B
        chnotes.third = 15; // Em
        chnotes.fifth = 18; // Gm
    }

    return chnotes;
}
