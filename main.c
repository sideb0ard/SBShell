#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include "audioutils.h"
#include "cmdloop.h"
#include "defjams.h"
#include "envelope.h"
#include "midimaaan.h"
#include "mixer.h"

mixer *mixr;

// use broadcast to wake up threads when midi tick changes
pthread_cond_t midi_tick_cond;
pthread_mutex_t midi_tick_lock;

static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData)
{
    paData *data = (paData *)userData;
    float *out = (float *)outputBuffer;
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        float val = 0;
        val = gen_next(data->mixr);
        *out++ = val;
        *out++ = val;
    }
    return 0;
}

int main()
{

    pthread_mutex_init(&midi_tick_lock, NULL);
    pthread_cond_init(&midi_tick_cond, NULL);

    srand(time(NULL));

    //// run the MIDI event looprrr...
    pthread_t midi_th;
    if (pthread_create(&midi_th, NULL, midiman, NULL)) {
        fprintf(stderr, "Errrr, wit tha midi..\n");
        return -1;
    }
    pthread_detach(midi_th);

    // PortAudio start me up!
    pa_setup();

    PaStream *stream;
    PaError err;
    paData *data = calloc(1, sizeof(paData));
    mixr = new_mixer();
    data->mixr = mixr;

    err = Pa_OpenDefaultStream(&stream,
                               0,         // no input channels
                               2,         // stereo output
                               paFloat32, // 32bit fp output
                               SAMPLE_RATE, paFramesPerBufferUnspecified,
                               paCallback, data);

    if (err != paNoError) {
        printf("Errrrr! couldn't open Portaudio default stream: %s\n",
               Pa_GetErrorText(err));
        exit(-1);
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("Errrrr! couldn't start stream: %s\n", Pa_GetErrorText(err));
        exit(-1);
    }

    loopy();

    // all done, time to go home
    pa_teardown();

    return 0;
}
