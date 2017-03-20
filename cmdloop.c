#include <locale.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "algorithm.h"
#include "audioutils.h"
#include "beatrepeat.h"
#include "chaosmonkey.h"
#include "cmdloop.h"
#include "defjams.h"
#include "drumr_utils.h"
#include "envelope.h"
#include "help.h"
#include "keys.h"
#include "midimaaan.h"
#include "minisynth.h"
#include "mixer.h"
#include "obliquestrategies.h"
#include "oscillator.h"
#include "sampler.h"
#include "sparkline.h"
#include "table.h"
#include "utils.h"

extern mixer *mixr;

extern wtable *wave_tables[5];

char const *prompt = ANSI_COLOR_MAGENTA "SB#> " ANSI_COLOR_RESET;

void print_prompt()
{
    printf("%s", prompt);
    fflush(stdout);
}

void loopy(void)
{
    setlocale(LC_ALL, "");

    char *line;
    while ((line = readline(prompt)) != NULL) {
        if (line[0] != 0) {
            add_history(line);
            interpret(line);
        }
        free(line);
    }
}

void interpret(char *line)
{
    char wurds[NUM_WURDS][SIZE_OF_WURD];

    char *cmd, *last_s;
    char const *sep = ",";
    for (cmd = strtok_r(line, sep, &last_s); cmd;
         cmd = strtok_r(NULL, sep, &last_s)) {

        char tmp[128];
        strncpy((char *)tmp, cmd, 127);

        int num_wurds = parse_wurds_from_cmd(wurds, tmp);

        // Understanding these commands should make more sense when viewed
        // against the help output, which details each command
        // (TODO) some kid of doxygen-like scheme to write help output here

        //////  MIXER COMMANDS  /////////////////////////
        if (strncmp("help", wurds[0], 4) == 0) {
            print_help();
        }

        else if (strncmp("quit", wurds[0], 4) == 0 ||
                 strncmp("exit", wurds[0], 4) == 0) {
            exxit();
        }

        else if (strncmp("bpm", wurds[0], 3) == 0) {
            int bpm = atoi(wurds[1]);
            if (bpm > 0)
                link_set_bpm(mixr->m_ableton_link, bpm);
            // mixer_update_bpm(mixr, bpm);
        }

        else if (strncmp("debug", wurds[0], 5) == 0) {
            if (strncmp("on", wurds[1], 2) == 0 ||
                strncmp("true", wurds[1], 4) == 0) {
                printf("Enabling debug mode\n");
                mixr->debug_mode = true;
            }
            else if (strncmp("off", wurds[1], 2) == 0 ||
                     strncmp("false", wurds[1], 5) == 0) {
                printf("Disabling debug mode\n");
                mixr->debug_mode = false;
            }
        }

        else if (strncmp("ps", wurds[0], 2) == 0) {
            mixer_ps(mixr);
        }

        else if (strncmp("quiet", wurds[0], 5) == 0) {
            for (int i = 0; i < mixr->soundgen_num; i++)
                mixr->sound_generators[i]->setvol(mixr->sound_generators[i],
                                                  0.0);
        }
        else if (strncmp("ls", wurds[0], 2) == 0) {
            list_sample_dir();
        }

        else if (strncmp("down", wurds[0], 4) == 0 ||
                 strncmp("up", wurds[0], 3) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                SBMSG *msg = new_sbmsg();
                msg->sound_gen_num = soundgen_num;
                if (strcmp("up", wurds[0]) == 0) {
                    strncpy(msg->cmd, "fadeuprrr", 19);
                }
                else {
                    strncpy(msg->cmd, "fadedownrrr", 19);
                }
                thrunner(msg);
            }
        }

        else if (strncmp("vol", wurds[0], 3) == 0) {
            if (strncmp("mixer", wurds[1], 5) == 0) {
                double vol = atof(wurds[2]);
                mixer_vol_change(mixr, vol);
            }
            else {
                int soundgen_num = atoi(wurds[1]);
                if (is_valid_soundgen_num(soundgen_num)) {
                    double vol = atof(wurds[2]);
                    vol_change(mixr, soundgen_num, vol);
                }
            }
        }

        //////  STEP SEQUENCER COMMANDS  /////////////////////////
        else if (strncmp("seq", wurds[0], 3) == 0) {

            char *pattern = (char *)calloc(128, sizeof(char));

            if (is_valid_file(wurds[1])) {
                char_array_to_seq_string_pattern(pattern, wurds, 2, num_wurds);
                add_drum_char_pattern(mixr, wurds[1], pattern);
            }
            else {
                int soundgen_num = atoi(wurds[1]);
                if (is_valid_soundgen_num(soundgen_num) &&
                    mixr->sound_generators[soundgen_num]->type == DRUM_TYPE) {

                    DRUM *d = (DRUM *)mixr->sound_generators[soundgen_num];

                    if (strncmp("add", wurds[2], 3) == 0) {
                        printf("Adding\n");
                        char_array_to_seq_string_pattern(pattern, wurds, 3,
                                                         num_wurds);
                        add_char_pattern(d, pattern);
                    }
                    else if (strncmp("randamp", wurds[2], 6) == 0) {
                        int pattern_num = atoi(wurds[3]);
                        if (is_valid_drum_pattern_num(d, pattern_num)) {
                            drum_set_random_sample_amp(d, pattern_num);
                        }
                    }
                    else if (strncmp("multi", wurds[2], 5) == 0) {
                        if (strncmp("true", wurds[3], 4) == 0) {
                            drumr_set_multi_pattern_mode(d, true);
                        }
                        else if (strncmp("false", wurds[3], 5) == 0) {
                            drumr_set_multi_pattern_mode(d, false);
                        }
                        printf("Sequencer multi mode : %s\n",
                               d->multi_pattern_mode ? "true" : "false");
                    }
                    else if (strncmp("change", wurds[2], 6) == 0) {
                        if (strncmp("markov", wurds[3], 6) == 0) {
                            printf("MARKOV!\n");
                            if (strncmp("haus", wurds[4], 4) == 0) {
                                printf("HAUS!\n");
                                seq_set_markov_mode(d, MARKOVHAUS);
                            }
                            else if (strncmp("boombap", wurds[4], 7) == 0) {
                                printf("BOOMBAP!\n");
                                seq_set_markov_mode(d, MARKOVBOOMBAP);
                            }
                        }
                        else {
                            // TODO -- this looks fragile - only used in two of
                            // the below situs
                            char_array_to_seq_string_pattern(pattern, wurds, 5,
                                                             num_wurds);
                            printf("Changing\n");

                            int pattern_num = atoi(wurds[3]);
                            if (is_valid_drum_pattern_num(d, pattern_num)) {
                                if (strncmp("pattern", wurds[4], 7) == 0) {
                                    printf("Changing pattern to %s\n", pattern);
                                    change_char_pattern(d, pattern_num,
                                                        pattern);
                                }
                                else if (strncmp("amp", wurds[4], 3) == 0) {
                                    printf("Setting pattern AMP to %s\n",
                                           pattern);
                                    drum_set_sample_amp_from_char_pattern(
                                        d, pattern_num, pattern);
                                }
                                else if (strncmp("numloops", wurds[4], 8) ==
                                         0) {
                                    int numloops = atoi(wurds[5]);
                                    if (numloops != 0) {
                                        drumr_change_num_loops(d, pattern_num,
                                                               numloops);
                                    }
                                }
                            }
                        }
                    }
                    else if (strncmp("euclid", wurds[2], 6) == 0) {
                        // https://en.wikipedia.org/wiki/Euclidean_rhythm
                        int num_beats = atoi(wurds[3]);
                        if (num_beats <= 0) {
                            return;
                        }
                        int euclidean_pattern = create_euclidean_rhythm(
                            num_beats, DRUM_PATTERN_LEN);
                        bool start_at_zero =
                            strncmp("true", wurds[4], 4) == 0 ? true : false;
                        if (start_at_zero)
                            euclidean_pattern = shift_bits_to_leftmost_position(
                                euclidean_pattern, DRUM_PATTERN_LEN);
                        change_int_pattern(d, d->cur_pattern,
                                           euclidean_pattern);
                    }
                    else if (strncmp("life", wurds[2], 4) == 0) {
                        int num_gens = atoi(wurds[3]);
                        if (num_gens > 0) {
                            if (mixr->debug_mode)
                                printf("Enabling game of life for %d "
                                       "generations\n",
                                       num_gens);
                            seq_set_game_of_life(d, 1);
                            seq_set_max_generations(d, num_gens);
                        }
                        else {
                            printf("Toggling game of life for %d\n",
                                   soundgen_num);
                            seq_set_game_of_life(d, 1 - d->game_of_life_on);
                        }
                    }
                    else if (strncmp("markov", wurds[2], 4) == 0) {
                        int num_gens = atoi(wurds[3]);
                        if (num_gens > 0) {
                            if (mixr->debug_mode)
                                printf(
                                    "Enabling Markov mode for %d generations\n",
                                    num_gens);
                            seq_set_markov(d, 1);
                            seq_set_max_generations(d, num_gens);
                        }
                        else {
                            printf("Toggling Markov for %d\n", soundgen_num);
                            seq_set_markov(d, 1 - d->markov_on);
                        }
                    }
                    else if (strncmp("swing", wurds[2], 5) == 0) {
                        int swing_setting = atoi(wurds[3]);
                        swingrrr(mixr->sound_generators[soundgen_num],
                                 swing_setting);
                    }
                }
            }
            free(pattern);
        }

        // BITSHIFT MACHINEZ
        else if (strncmp("bitshift", wurds[0], 8) == 0) {
            int bitpattern = atoi(wurds[1]);
            printf("BUTTSHIFTZ! %d\n", bitpattern);
            add_bitwize(mixr, bitpattern);
        }

        // SAMPLE LOOPER COMMANDS
        else if (strncmp("loop", wurds[0], 4) == 0) {
            if (is_valid_file(wurds[1]) || strncmp(wurds[1], "none", 4) == 0) {
                int loop_len = atoi(wurds[2]);
                if (loop_len > 0) {
                    add_sampler(mixr, wurds[1], loop_len);
                }
            }
            else {
                int soundgen_num = atoi(wurds[1]);
                if (is_valid_soundgen_num(soundgen_num) &&
                    mixr->sound_generators[soundgen_num]->type ==
                        SAMPLER_TYPE) {

                    SAMPLER *s =
                        (SAMPLER *)mixr->sound_generators[soundgen_num];

                    if (strncmp("add", wurds[2], 6) == 0) {
                        if (is_valid_file(wurds[3]) ||
                            strncmp(wurds[3], "none", 4) == 0) {
                            int loop_len = atoi(wurds[4]);
                            if (loop_len > 0) {
                                sampler_add_sample(s, wurds[3], loop_len);
                            }
                        }
                    }
                    else if (strncmp("scramble", wurds[2], 8) == 0) {
                        if (strncmp(wurds[3], "true", 4) == 0)
                            sampler_set_scramble_mode(s, true);
                        else if (strncmp(wurds[3], "false", 5) == 0)
                            sampler_set_scramble_mode(s, false);
                        else {
                            int max_gen = atoi(wurds[3]);
                            if (max_gen > 0) {
                                sampler_set_max_generation(s, max_gen);
                                sampler_set_scramble_mode(s, true);
                            }
                            else {
                                printf("Toggling scramble..\n");
                                int new_mode = 1 - s->scramblrrr_mode;
                                sampler_set_scramble_mode(s, new_mode);
                            }
                        }
                    }
                    else if (strncmp("stutter", wurds[2], 7) == 0) {
                        if (strncmp(wurds[3], "true", 4) == 0)
                            sampler_set_stutter_mode(s, true);
                        else if (strncmp(wurds[3], "false", 5) == 0)
                            sampler_set_stutter_mode(s, false);
                        else {
                            int max_gen = atoi(wurds[3]);
                            if (max_gen > 0) {
                                sampler_set_max_generation(s, max_gen);
                                sampler_set_stutter_mode(s, true);
                            }
                            else {
                                printf("Toggling sTUTTER..\n");
                                int new_mode = 1 - s->stutter_mode;
                                sampler_set_stutter_mode(s, new_mode);
                            }
                        }
                    }
                    else if (strncmp("multi", wurds[2], 5) == 0) {
                        if (strncmp("true", wurds[3], 4) == 0) {
                            sampler_set_multi_sample_mode(s, true);
                        }
                        else if (strncmp("false", wurds[3], 5) == 0) {
                            sampler_set_multi_sample_mode(s, false);
                        }
                        printf("Sampler multi mode : %s\n",
                               s->multi_sample_mode ? "true" : "false");
                    }
                    else if (strncmp("change", wurds[2], 6) == 0) {
                        int sample_num = atoi(wurds[3]);
                        if (is_valid_sample_num(s, sample_num)) {
                            if (strncmp("looplen", wurds[4], 8) == 0) {
                                int looplen = atoi(wurds[5]);
                                sampler_change_loop_len(s, sample_num, looplen);
                            }
                            else if (strncmp("numloops", wurds[4], 8) == 0) {
                                int numloops = atoi(wurds[5]);
                                if (numloops != 0) {
                                    sampler_change_num_loops(s, sample_num,
                                                             numloops);
                                }
                            }
                        }
                    }
                    else if (strncmp("switch", wurds[2], 6) == 0) {
                        int sample_num = atoi(wurds[3]);
                        sampler_switch_sample(s, sample_num);
                    }
                }
            }
        }

        // SINGLE SHOT SAMPLE PLAYER COMMANDS
        else if (strncmp("play", wurds[0], 4) == 0) {
            printf("Playing onetime sample...\n");
        }

        // SYNTHESIZER COMMANDS
        else if (strncmp("syn", wurds[0], 3) == 0) {
            if (strncmp("mini", wurds[1], 4) == 0) {
                add_minisynth(mixr);
            }
            else {
                int soundgen_num = atoi(wurds[1]);
                if (is_valid_soundgen_num(soundgen_num) &&
                    mixr->sound_generators[soundgen_num]->type == SYNTH_TYPE) {
                    minisynth *ms =
                        (minisynth *)mixr->sound_generators[soundgen_num];

                    if (strncmp("add", wurds[2], 4) == 0) {
                        if (strncmp("melody", wurds[3], 6) == 0) {
                            minisynth_add_melody(ms);
                        }
                    }
                    if (strncmp("multi", wurds[2], 5) == 0) {
                        if (strncmp("true", wurds[3], 4) == 0) {
                            minisynth_set_multi_melody_mode(ms, true);
                        }
                        else if (strncmp("false", wurds[3], 5) == 0) {
                            minisynth_set_multi_melody_mode(ms, false);
                        }
                        printf("Synth multi mode : %s\n",
                               ms->multi_melody_mode ? "true" : "false");
                    }
                    if (strncmp("sustain", wurds[2], 5) == 0) {
                        if (strncmp("true", wurds[3], 4) == 0) {
                            minisynth_set_sustain_override(ms, true);
                        }
                        else if (strncmp("false", wurds[3], 5) == 0) {
                            minisynth_set_sustain_override(ms, false);
                        }
                        printf("Synth Sustain Override : %s\n",
                               ms->m_sustain_override ? "true" : "false");
                        for (int i = 0; i < MAX_VOICES; i++) {
                            if (ms->m_voices[i]) {
                                printf("EG sustain: %d\n",
                                       ms->m_voices[i]
                                           ->m_voice.m_eg1.m_sustain_override);
                                printf("EG sustain: %d\n",
                                       ms->m_voices[i]
                                           ->m_voice.m_eg2.m_sustain_override);
                                printf("EG sustain: %d\n",
                                       ms->m_voices[i]
                                           ->m_voice.m_eg3.m_sustain_override);
                                printf("EG sustain: %d\n",
                                       ms->m_voices[i]
                                           ->m_voice.m_eg4.m_sustain_override);
                            }
                        }
                    }
                    else if (strncmp("change", wurds[2], 6) == 0) {
                        int melody_num = atoi(wurds[3]);
                        if (is_valid_melody_num(ms, melody_num)) {
                            printf("VALID!\n");
                            if (strncmp("numloops", wurds[4], 8) == 0) {
                                int numloops = atoi(wurds[5]);
                                if (numloops != 0) {
                                    minisynth_set_melody_loop_num(
                                        ms, melody_num, numloops);
                                    printf("NUMLOOPS Now %d\n", numloops);
                                }
                            }
                        }
                    }
                    else if (strncmp("copy", wurds[2], 4) == 0) {
                        int sg2 = atoi(wurds[3]);
                        if (is_valid_soundgen_num(sg2) &&
                            mixr->sound_generators[sg2]->type == SYNTH_TYPE) {
                            minisynth *ms2 =
                                (minisynth *)mixr->sound_generators[sg2];
                            printf("Copying SYNTH patterns!\n");
                            int pattern_nomnom = atoi(wurds[4]);
                            if (pattern_nomnom <= ms2->num_melodies) {
                                printf("And we got patterns to copy~!\n");
                            }
                        }
                        // if wurds[3] is valid soundnum and wurds[4] <=
                        // wurds[3]->num_patterns
                        // wurds[1]->add_pattern
                        // wurds[1]->pattern = wurds[3]->get_pattern(wurds[4])
                    }
                    else if (strncmp("keys", wurds[2], 4) == 0) {
                        keys(soundgen_num);
                    }
                    else if (strncmp("midi", wurds[2], 4) == 0) {
                        mixr->midi_control_destination = SYNTH;
                        mixr->active_midi_soundgen_num = soundgen_num;
                    }
                    else if (strncmp("quantize", wurds[2], 8) == 0) {
                        int melody_num = atoi(wurds[3]);
                        if (is_valid_melody_num(ms, melody_num)) {
                            printf("QuantiZe!\n");
                            midi_event **melody = ms->melodies[ms->cur_melody];
                            midi_melody_quantize(melody);
                        }
                    }
                    else if (strncmp("reset", wurds[2], 5) == 0) {
                        if (strncmp("all", wurds[3], 3) == 0) {
                            minisynth_reset_melody_all(ms);
                        }
                        else {
                            int melody_num = atoi(wurds[3]);
                            minisynth_reset_melody(ms, melody_num);
                        }
                    }
                    else if (strncmp("switch", wurds[2], 6) == 0) {
                        int melody_num = atoi(wurds[3]);
                        minisynth_switch_melody(ms, melody_num);
                    }
                }
            }
        }

        // CHAOS COMMANDS
        else if (strncmp("chaos", wurds[0], 6) == 0) {

            if (strncmp("monkey", wurds[1], 6) == 0) {
                add_chaosmonkey();
            }
            else {
                int soundgen_num = atoi(wurds[1]);
                if (is_valid_soundgen_num(soundgen_num) &&
                    mixr->sound_generators[soundgen_num]->type ==
                        CHAOSMONKEY_TYPE) {

                    chaosmonkey *cm =
                        (chaosmonkey *)mixr->sound_generators[soundgen_num];

                    if (strncmp("wakeup", wurds[2], 7) == 0) {
                        int freq_wakeup_secs = atoi(wurds[3]);
                        chaosmonkey_change_wakeup_freq(cm, freq_wakeup_secs);
                    }
                    else if (strncmp("chance", wurds[2], 7) == 0) {
                        int percent_chance = atoi(wurds[3]);
                        chaosmonkey_change_chance_interrupt(cm, percent_chance);
                    }
                    else if (strncmp("suggest", wurds[2], 8) == 0) {
                        if (strncmp("true", wurds[3], 5) == 0 ||
                            strncmp("false", wurds[3], 6) == 0) {
                            bool val =
                                strcmp("true", wurds[4]) == 0 ? true : false;
                            chaosmonkey_suggest_mode(cm, val);
                        }
                    }
                    else if (strncmp("action", wurds[2], 7) == 0) {
                        if (strncmp("true", wurds[3], 5) == 0 ||
                            strncmp("false", wurds[3], 6) == 0) {
                            bool val =
                                strcmp("true", wurds[4]) == 0 ? true : false;
                            chaosmonkey_action_mode(cm, val);
                        }
                    }
                }
            }
        }
        // FX COMMANDS
        else if (strncmp("delay", wurds[0], 7) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                int delay_len_ms = atoi(wurds[2]);
                add_delay_soundgen(mixr->sound_generators[soundgen_num],
                                   delay_len_ms);
            }
        }
        else if (strncmp("moddelay", wurds[0], 7) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                add_moddelay_soundgen(mixr->sound_generators[soundgen_num]);
            }
        }
        else if (strncmp("reverb", wurds[0], 6) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                add_reverb_soundgen(mixr->sound_generators[soundgen_num]);
            }
        }
        else if (strncmp("distort", wurds[0], 7) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                add_distortion_soundgen(mixr->sound_generators[soundgen_num]);
            }
        }
        else if (strncmp("decimate", wurds[0], 8) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                add_decimator_soundgen(mixr->sound_generators[soundgen_num]);
            }
        }
        else if (strncmp("env", wurds[0], 3) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                int loop_len = atoi(wurds[2]);
                int env_type = atoi(wurds[3]);
                ENVSTREAM *e = new_envelope_stream(loop_len, env_type);
                if (e != NULL) {
                    add_envelope_soundgen(mixr->sound_generators[soundgen_num],
                                          e);
                }
            }
        }
        else if (strncmp("lowpass", wurds[0], 8) == 0 ||
                 strncmp("highpass", wurds[0], 9) == 0 ||
                 strncmp("bandpass", wurds[0], 9) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                int val = atoi(wurds[2]);
                if (strcmp("lowpass", wurds[0]) == 0)
                    add_freq_pass_soundgen(mixr->sound_generators[soundgen_num],
                                           val, LOWPASS);
                else if (strcmp("highass", wurds[0]) == 0)
                    add_freq_pass_soundgen(mixr->sound_generators[soundgen_num],
                                           val, HIGHPASS);
                else
                    add_freq_pass_soundgen(mixr->sound_generators[soundgen_num],
                                           val, BANDPASS);
            }
        }

        else if (strncmp("repeat", wurds[0], 6) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                int loop_len = atoi(wurds[2]);
                if (loop_len > 0) {
                    add_beatrepeat_soundgen(
                        mixr->sound_generators[soundgen_num], loop_len);
                }
            }
        }
        else if (strncmp("sidechain", wurds[0], 9) == 0) {
            int soundgen_num = atoi(wurds[1]);
            if (is_valid_soundgen_num(soundgen_num)) {
                int input_src = atoi(wurds[2]);
                int percent_mix = atoi(wurds[3]);
                printf("SIDEHCINA %d %d %d\n", soundgen_num, input_src,
                       percent_mix);
                // if (mixr->sound_generators[input_src]->type == DRUM_TYPE) {
                //    DRUM *d = (DRUM *)
                //    mixr->sound_generators[input_src]->type;
                //    int pat_array[DRUM_PATTERN_LEN];
                //    int_pattern_to_array(d->patterns[d->cur_pattern],
                //                         pat_array);
                //    for (int i = 0; i < DRUM_PATTERN_LEN; i++)
                //    {
                //        printf("DRUMMMMM %d\n", pat_array[0]);
                //    }
                //    //ENVSTREAM *e = new_sidechain_stream(pat_array,
                //    percent_mix);
                //    //printf("GOT STREAM\n");
                //    //add_envelope_soundgen(mixr->sound_generators[soundgen_num],
                //    e);
                //}
            }
        }

        else if (strncmp("fx", wurds[0], 2) == 0) {
            int soundgen_num = atoi(wurds[1]);
            int fx_num = atoi(wurds[2]);
            if (is_valid_fx_num(soundgen_num, fx_num)) {
                if (strncmp("nbeats", wurds[3], 6) == 0 ||
                    strncmp("16th", wurds[3], 4) == 0) {
                    if (mixr->sound_generators[soundgen_num]
                            ->effects[fx_num]
                            ->type == BEATREPEAT) {
                        beatrepeat *b =
                            (beatrepeat *)mixr->sound_generators[soundgen_num]
                                ->effects[fx_num];
                        if (strncmp("nbeats", wurds[3], 6) == 0) {
                            int nbeats = atoi(wurds[4]);
                            beatrepeat_change_num_beats_to_repeat(b, nbeats);
                        }
                        else {
                            int s16th = atoi(wurds[4]);
                            beatrepeat_change_selected_sixteenth(b, s16th);
                        }
                    }
                }
                else if (strncmp("modtype", wurds[3], 7) == 0 ||
                         strncmp("lfotype", wurds[3], 7) == 0) {
                    if (mixr->sound_generators[soundgen_num]
                            ->effects[fx_num]
                            ->type == MODDELAY) {
                        mod_delay *d =
                            (mod_delay *)mixr->sound_generators[soundgen_num]
                                ->effects[fx_num];
                        if (strncmp("modtype", wurds[3], 7) == 0) {
                            if (strncmp("flanger", wurds[4], 7) == 0)
                                d->m_mod_type = FLANGER;
                            if (strncmp("vibrato", wurds[4], 7) == 0)
                                d->m_mod_type = VIBRATO;
                            if (strncmp("chorus", wurds[4], 6) == 0)
                                d->m_mod_type = CHORUS;
                        }
                        else if (strncmp("lfotype", wurds[3], 7) == 0) {
                            if (strncmp("tri", wurds[4], 3) == 0)
                                d->m_lfo_type = TRI;
                            if (strncmp("sin", wurds[4], 3) == 0)
                                d->m_lfo_type = SINE;
                        }
                    }
                }
                else if (strncmp("midi", wurds[3], 4) == 0) {
                    if (mixr->sound_generators[soundgen_num]
                                ->effects[fx_num]
                                ->type == DELAY ||
                        mixr->sound_generators[soundgen_num]
                                ->effects[fx_num]
                                ->type == MODDELAY ||
                        mixr->sound_generators[soundgen_num]
                                ->effects[fx_num]
                                ->type == REVERB) {
                        printf("SUCCESS! GOLDEN MIDI DELAY!\n");
                        mixr->midi_control_destination =
                            DELAYFX; // TODO rename to FX
                        mixr->active_midi_soundgen_num = soundgen_num;
                        mixr->active_midi_soundgen_effect_num = fx_num;
                    }
                }
                else if (strncmp("wetmix", wurds[3], 6) == 0) {
                    if (mixr->sound_generators[soundgen_num]
                            ->effects[fx_num]
                            ->type == REVERB) {
                        reverb *r =
                            (reverb *)mixr->sound_generators[soundgen_num]
                                ->effects[fx_num]
                                ->r;
                        int wetmix = atoi(wurds[4]);
                        if (0 <= wetmix && wetmix <= 100)
                            r->m_wet_pct = wetmix;
                        if (mixr->debug_mode)
                            printf("REVERB WETMIX! %f\n", r->m_wet_pct);
                        reverb_cook_variables(r);
                    }
                }
                else if (strncmp("delay", wurds[3], 5) == 0 ||
                         strncmp("feedback", wurds[3], 8) == 0 ||
                         strncmp("ratio", wurds[3], 5) == 0 ||
                         strncmp("mix", wurds[3], 3) == 0 ||
                         strncmp("mode", wurds[3], 4) == 0) {
                    if (mixr->sound_generators[soundgen_num]
                            ->effects[fx_num]
                            ->type == DELAY) {
                        stereodelay *d =
                            (stereodelay *)mixr->sound_generators[soundgen_num]
                                ->effects[fx_num];
                        if (strncmp("delay", wurds[3], 5) == 0) {
                            if (mixr->debug_mode)
                                printf("Changing DELAY TIME\n");
                            double delay_ms = atof(wurds[4]);
                            stereo_delay_set_delay_time_ms(d, delay_ms);
                        }
                        else if (strncmp("feedback", wurds[3], 8) == 0) {
                            if (mixr->debug_mode)
                                printf("Changing FEEDBACK TIME\n");
                            int percent = atoi(wurds[4]);
                            stereo_delay_set_feedback_percent(d, percent);
                        }
                        else if (strncmp("ratio", wurds[3], 5) == 0) {
                            if (mixr->debug_mode)
                                printf("Changing RATIO TIME\n");
                            double ratio = atof(wurds[4]);
                            stereo_delay_set_delay_ratio(d, ratio);
                        }
                        else if (strncmp("mix", wurds[3], 3) == 0) {
                            if (mixr->debug_mode)
                                printf("Changing MIX TIME\n");
                            double mix = atof(wurds[4]);
                            stereo_delay_set_wet_mix(d, mix);
                        }
                        else if (strncmp("mode", wurds[3], 4) == 0) {
                            if (mixr->debug_mode)
                                printf("MODE!\n");
                            if (strncmp("NORM", wurds[4], 4) == 0) {
                                stereo_delay_set_mode(d, NORM);
                            }
                            else if (strncmp("TAP1", wurds[4], 4) == 0) {
                                stereo_delay_set_mode(d, TAP1);
                            }
                            else if (strncmp("TAP2", wurds[4], 4) == 0) {
                                stereo_delay_set_mode(d, TAP2);
                            }
                            else if (strncmp("PINGPONG", wurds[4], 8) == 0) {
                                stereo_delay_set_mode(d, PINGPONG);
                            }
                        }
                    }
                }
            }
        }

        // PROGRAMMING CMDS
        else if (strncmp("var", wurds[0], 3) == 0 &&
                 strncmp("=", wurds[2], 1) == 0) {
            printf("Oooh! %s = %s\n", wurds[1], wurds[3]);
            update_environment(wurds[1], atoi(wurds[3]));
        }

        else if (strncmp("every", wurds[0], 5) == 0 &&
                 strncmp("loop", wurds[1], 4) == 0) {
            printf("Starting an algorithm - with %s!\n", cmd);
            add_algorithm(cmd);
        }

        // UTILS
        else if (strncmp("chord", wurds[0], 6) == 0) {
            chordie(wurds[1]);
        }

        else if (strncmp("strategy", wurds[0], 8) == 0) {
            oblique_strategy();
        }

        // default HALP!
        else {
            print_help();
        }
    }
}

int parse_wurds_from_cmd(char wurds[][SIZE_OF_WURD], char *line)
{
    memset(wurds, 0, NUM_WURDS * SIZE_OF_WURD);
    int num_wurds = 0;
    char const *sep = " ";
    char *tok, *last_s;
    for (tok = strtok_r(line, sep, &last_s); tok;
         tok = strtok_r(NULL, sep, &last_s)) {
        strncpy(wurds[num_wurds++], tok, SIZE_OF_WURD);
        if (num_wurds == NUM_WURDS)
            break;
    }
    return num_wurds;
}

void char_array_to_string_sequence(char *dest_pattern,
                                   char char_array[NUM_WURDS][SIZE_OF_WURD],
                                   int start, int end)
{
    for (int i = start; i < end; i++) {
        strcat(dest_pattern, char_array[i]);
        if (i != (end - 1))
            strcat(dest_pattern, " ");
    }
}

void char_array_to_seq_string_pattern(char *dest_pattern,
                                      char char_array[NUM_WURDS][SIZE_OF_WURD],
                                      int start, int end)
{
    if (strncmp("all", char_array[start], 3) == 0) {
        strncat(dest_pattern, "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15", 127);
    }
    else if (strncmp("none", char_array[start], 4) == 0) {
        // no-op
    }
    else {
        for (int i = start; i < end; i++) {
            strcat(dest_pattern, char_array[i]);
            if (i != (end - 1))
                strcat(dest_pattern, " ");
        }
        // char_array_to_seq_string_pattern(dest_pattern, char_array, start,
        // end);
    }
}

bool is_valid_soundgen_num(int soundgen_num)
{
    if (soundgen_num >= 0 && soundgen_num < mixr->soundgen_num) {
        return true;
    }
    return false;
}

bool is_valid_sample_num(SAMPLER *s, int sample_num)
{
    if (sample_num < s->num_samples) {
        return true;
    }
    return false;
}

bool is_valid_drum_pattern_num(DRUM *d, int pattern_num)
{
    if (pattern_num < d->num_patterns) {
        return true;
    }
    return false;
}

bool is_valid_melody_num(minisynth *ms, int melody_num)
{
    if (melody_num < ms->num_melodies) {
        return true;
    }
    return false;
}

bool is_valid_fx_num(int soundgen_num, int fx_num)
{
    if (is_valid_soundgen_num(soundgen_num)) {
        if (mixr->sound_generators[soundgen_num]->effects_num > 0 &&
            fx_num < mixr->sound_generators[soundgen_num]->effects_num) {
            return true;
        }
    }
    return false;
}

bool is_valid_file(char *filename)
{
    char cwd[1024];
    getcwd(cwd, 1024);
    char *subdir = "/wavs/";
    char full_filename[strlen(cwd) + strlen(filename) + strlen(subdir)];
    strcpy(full_filename, cwd);
    strcat(full_filename, subdir);
    strcat(full_filename, filename);

    struct stat buffer;
    return (stat(full_filename, &buffer) == 0);
}

int exxit()
{
    printf(COOL_COLOR_GREEN "\nBeat it, ya val jerk...\n" ANSI_COLOR_RESET);
    pa_teardown();
    exit(0);
}
