#include "help.h"
#include "defjams.h"
#include <stdio.h>

void print_help()
{
    printf("\n" COOL_COLOR_PINK "#### SBShell - Interactive, scriptable, "
           "algorithmic music shell ####\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[Global Cmds]\n");
    // printf(ANSI_COLOR_WHITE "stop" ANSI_COLOR_CYAN " -- stop playback\n");
    // printf(ANSI_COLOR_WHITE "start" ANSI_COLOR_CYAN " -- re-start
    // playback\n");
    printf(ANSI_COLOR_WHITE "bpm <bpm>" ANSI_COLOR_CYAN
                            " -- change bpm to <bpm>\n");
    printf(ANSI_COLOR_WHITE "vol <soundgen_num> <vol>" ANSI_COLOR_CYAN
                            " -- change volume of Soundgenerator to <vol>\n");
    printf(ANSI_COLOR_WHITE "vol mixer <vol>" ANSI_COLOR_CYAN
                            " -- change mixer volume to <vol>\n");
    printf(ANSI_COLOR_WHITE "ps" ANSI_COLOR_CYAN
                            " -- show global status and process listing\n");
    printf(ANSI_COLOR_WHITE
           "ls" ANSI_COLOR_CYAN
           " -- show file listing of samples, loops, and file projects\n");
    // printf(ANSI_COLOR_WHITE
    //        "save <filename>" ANSI_COLOR_CYAN
    //        " -- save current project settings as <filename>\n");
    // printf(ANSI_COLOR_WHITE
    //        "open <filename>" ANSI_COLOR_CYAN
    //        " -- open <filename> as current project. (Overrides "
    //        "current)\n");
    // printf(ANSI_COLOR_WHITE "record <on/off>" ANSI_COLOR_CYAN
    //                         " -- toggle global record on or off\n");
    printf(ANSI_COLOR_WHITE "help" ANSI_COLOR_CYAN " -- this message, duh\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[Sample Looper Cmds]\n");
    printf(ANSI_COLOR_WHITE "loop <sample> <bars>" ANSI_COLOR_CYAN
                            " e.g. \"loop amen.wav 2\"\n");
    printf(ANSI_COLOR_WHITE "loop <sound_gen_no> add <sample> <bars>\n");
    printf(ANSI_COLOR_WHITE "loop <sound_gen_no> change <parameter> <val>\n");
    printf("\n");
    // printf(ANSI_COLOR_WHITE "play <sample> [16th]" ANSI_COLOR_CYAN
    //                         " -- play one-shot sample. Optional 16th to
    //                         start, "
    //                         "otherwise plays immediately\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[Step Sequencer Cmds]\n");
    printf(ANSI_COLOR_WHITE);
    printf(ANSI_COLOR_WHITE "seq <sample> <pattern>" ANSI_COLOR_CYAN
                            " e.g. \"seq kick2.wav 0 4 8 12\"\n");
    printf(ANSI_COLOR_WHITE "seq <sound_gen_no> add <pattern>" ANSI_COLOR_CYAN
                            " e.g. seq 0 add 4 6 10 12\n");
    printf(ANSI_COLOR_WHITE "seq <sound_gen_no> change <pattern>\n");
    printf(ANSI_COLOR_WHITE
           "seq <sound_gen_no> euclid <num_hits> [true]" ANSI_COLOR_CYAN
           "\n   -- generates "
           "equally spaced number of beats. Optional 'true' shifts\n    them "
           "forward so first hit is on first tick of cycle.\n");
    printf(ANSI_COLOR_WHITE "seq <sound_gen_no> life" ANSI_COLOR_CYAN
                            " - generative changing pattern, based on "
                            "game of life\n");
    printf(ANSI_COLOR_WHITE
           "seq <sound_gen_no> swing <swing_setting>" ANSI_COLOR_CYAN
           "\n    -- toggles swing"
           " on/off. Setting can be between 1..6, which represent\n"
           "    50%%, 54%%, 58%%, 62%%, 66%%, 71%%\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[Synthesizer Cmds]\n");
    printf(ANSI_COLOR_WHITE "syn nano" ANSI_COLOR_CYAN
                            " -- start new Nano Synth\n");
    // printf(ANSI_COLOR_WHITE "syn korg" ANSI_COLOR_CYAN
    //                         " -- start new Korg Synth\n");
    // printf(ANSI_COLOR_WHITE "syn poly" ANSI_COLOR_CYAN
    //                         " -- start new Poly Synth\n");
    printf(ANSI_COLOR_WHITE "syn <sound_gen_no> keys" ANSI_COLOR_CYAN
                            " -- control synth via keyboard\n");
    printf(ANSI_COLOR_WHITE "syn <sound_gen_no> midi" ANSI_COLOR_CYAN
                            " -- control synth via midi controller\n");
    printf(ANSI_COLOR_WHITE "syn <sound_gen_no> change <parameter> <val>\n");
    printf(ANSI_COLOR_WHITE "syn <sound_gen_no> reset" ANSI_COLOR_CYAN
                            " -- clear pattern data\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[FX Cmds]\n");
    printf(ANSI_COLOR_WHITE "decimate  <sound_gen_number>\n");
    // printf(ANSI_COLOR_WHITE "delay     <sound_gen_number>\n");
    // printf(ANSI_COLOR_WHITE "distort   <sound_gen_number>\n");
    printf(ANSI_COLOR_WHITE "env       <sound_gen_number> <loop_len> <type>\n");
    // printf(ANSI_COLOR_WHITE "reverb    <sound_gen_number>\n");
    // printf(ANSI_COLOR_WHITE "crush     <sound_gen_number>\n");
    printf(ANSI_COLOR_WHITE
           "repeat    <sound_gen_number> <loop_len>" ANSI_COLOR_CYAN
           " -- beatrepeat, default settings\n");
    printf(ANSI_COLOR_WHITE
           "sidechain <sound_gen_number> <input_src> <mix>" ANSI_COLOR_CYAN
           " -- sidechain env, where input_src is a drum pattern\n");
    printf(ANSI_COLOR_WHITE
           "fx        <sound_gen_number> <fx_num> <parameter> <val>\n");

    /////////////////////////////////////////////////////////////////////
    printf("\n" COOL_COLOR_GREEN "[Programming Cmds]\n");
    printf(ANSI_COLOR_WHITE "var <var_name> = <value>" ANSI_COLOR_CYAN
                            " -- store global variable\n");
    printf(ANSI_COLOR_WHITE "every loop; <cmd1> ; <cmd2>; ..." ANSI_COLOR_CYAN
                            " -- run cmds once every loop\n");

    printf(ANSI_COLOR_RESET);
}
