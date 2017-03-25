#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "defjams.h"
#include "keys.h"
#include "midimaaan.h"
#include "mixer.h"
#include "utils.h"

extern mixer *mixr;
extern pthread_cond_t midi_tick_cond;
extern pthread_mutex_t midi_tick_lock;

void keys(int soundgen_num)
{
    printf("Entering Keys Mode for %d\n", soundgen_num);
    printf("Press 'q' or 'Esc' to go back to Run Mode\n");
    struct termios new_info, old_info;
    tcgetattr(0, &old_info);
    new_info = old_info;

    new_info.c_lflag &= (~ICANON & ~ECHO);
    new_info.c_cc[VMIN] = 1;
    new_info.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_info);

    minisynth *ms = (minisynth *)mixr->sound_generators[soundgen_num];

    int ch = 0;
    int quit = 0;

    struct pollfd fdz[1];
    int pollret;

    fdz[0].fd = STDIN_FILENO;
    fdz[0].events = POLLIN;

    while (!quit) {

        pollret = poll(fdz, 1, 0);
        if (pollret == -1) {
            perror("poll");
            return;
        }

        if (fdz[0].revents & POLLIN) {
            ch = getchar();
            printf("C %d\n", ch);
            int midi_num;
            switch (ch) {
            case 27:
            case 113:
                quit = 1;
                break;
            case 49:
                printf("Down an octave...\n");
                // change_octave(ns, DOWN);
                break;
            case 50:
                printf("Up an octave...\n");
                // change_octave(ns, UP);
                break;
            case 114:
                ms->recording = 1 - ms->recording;
                printf("Toggling REC to %s\n",
                       ms->recording ? "true" : "false");
                break;
            case 122:
                printf("SAW3 mode\n");
                ms->m_voice_mode = 0;
                break;
            case 120:
                printf("SQR3 mode\n");
                ms->m_voice_mode = 1;
                break;
            case 99:
                printf("SAW2SQR mode\n");
                ms->m_voice_mode = 2;
                break;
            case 118:
                printf("TRI2SAW mode\n");
                ms->m_voice_mode = 3;
                break;
            case 98:
                printf("TRI2SQR mode\n");
                ms->m_voice_mode = 4;
                break;
            case 110:
                minisynth_toggle_delay_mode(ms);
                printf("Switching DELAY mode -- %d\n", ms->m_delay_mode);
                break;
            case 109:
                mixer_toggle_midi_mode(mixr);
                printf("Switching MIDI mode -- %d\n",
                       mixr->m_midi_controller_mode);
                break;
            default:
                // play note
                midi_num = ch_midi_lookup(ch, ms);
                int fake_velocity = 126; // TODO real velocity
                if (midi_num != -1) {

                    print_midi_event(midi_num);
                    minisynth_midi_note_on(ms, midi_num, fake_velocity);

                    int note_off_tick =
                        ((mixr->midi_tick % PPNS) + PPS * 4) %
                        PPNS; // rough guess - PPS is pulses per quart note
                              // and PPNS is pulses per minisynth Loop
                    midi_event *ev = new_midi_event(note_off_tick, 128,
                                                    midi_num, fake_velocity);
                    ev->delete_after_use = true; // _THIS_ is the magic
                    // ms->melodies[ms->cur_melody][note_off_tick] = ev;
                    minisynth_add_event(ms, ev);
                    ////////////////////////

                    if (ms->recording) {
                        printf("Recording note!\n");
                        int note_on_tick = mixr->midi_tick % PPNS;
                        midi_event *ev = new_midi_event(
                            note_on_tick, 144, midi_num, fake_velocity);

                        ms->melodies[ms->cur_melody][note_on_tick] = ev;

                        int note_off_tick =
                            (note_on_tick + PPS * 4) %
                            PPNS; // rough guess - PPS is pulses per quart note
                                  // and PPNS is pulses per minisynth Loop
                        midi_event *ev2 = new_midi_event(
                            note_off_tick, 128, midi_num, fake_velocity);
                        minisynth_add_event(ms, ev2);
                        // ms->melodies[ms->cur_melody][note_off_tick] = ev2;
                    }
                }
                printf("CCCC %d\n", ch);
            }
            minisynth_update(ms);
        }
    }
    tcsetattr(0, TCSANOW, &old_info);
}

void *play_melody_loop(void *p)
{
    minisynth *ms = (minisynth *)p;

    printf("PLAY melody starting..\n");

    while (1) {
        pthread_mutex_lock(&midi_tick_lock);
        pthread_cond_wait(&midi_tick_cond, &midi_tick_lock);
        pthread_mutex_unlock(&midi_tick_lock);

        int idx = mixr->midi_tick % PPNS;

        // top of the loop, check if we need to progress to next loop
        if (idx == 0) {
            if (ms->multi_melody_mode) {
                ms->cur_melody_iteration--;
                if (ms->cur_melody_iteration == 0) {
                    ms->cur_melody = (ms->cur_melody + 1) % ms->num_melodies;
                    ms->cur_melody_iteration =
                        ms->melody_multiloop_count[ms->cur_melody];
                    // TODO - this doesn't seem to have intended effect
                    // minisynth_reset_voices(ms);
                }
            }
        }

        if (ms->melodies[ms->cur_melody][idx] != NULL) {
            midi_event *ev = ms->melodies[ms->cur_melody][idx];
            midi_parse_midi_event(ms, ev);
        }
    }

    return NULL;
}
