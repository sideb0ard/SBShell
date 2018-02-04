#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

#include "algorithm.h"
#include "basicfilterpass.h"
#include "beatrepeat.h"
#include "bitcrush.h"
#include "chaosmonkey.h"
#include "defjams.h"
#include "distortion.h"
#include "dynamics_processor.h"
#include "envelope.h"
#include "envelope_follower.h"
#include "euclidean.h"
#include "mixer.h"
#include "modfilter.h"
#include "modular_delay.h"
#include "reverb.h"
#include "sequencer_utils.h"
#include "sparkline.h"
#include "table.h"
#include "utils.h"
#include "waveshaper.h"
#include <stereodelay.h>

#include <pattern_cmds.h>

extern mixer *mixr;

bool parse_pattern_cmd(int num_wurds, char wurds[][SIZE_OF_WURD])
{
    if (strncmp("pattern", wurds[0], 7) == 0)
    {
        int sgnum = atoi(wurds[1]);
        if (mixer_is_valid_seq_gen_num(mixr, sgnum))
        {
            sequence_generator *sg = mixr->sequence_generators[sgnum];
            if (strncmp("gen", wurds[2], 3) == 0)
            {
                int num =
                    sg->generate(sg, (void *)&mixr->timing_info.cur_sample);
                char binnum[33] = {0};
                char_binary_version_of_int(num, binnum);
                printf("NOM!: %d %s\n", num, binnum);
            }
            if (sg->type == EUCLIDEAN)
            {
                euclidean *e = (euclidean *)sg;
                if (strncmp(wurds[2], "mode", 4) == 0)
                {
                    int mode = atoi(wurds[3]);
                    euclidean_change_mode(e, mode);
                }
                if (strncmp(wurds[2], "hits", 4) == 0)
                {
                    int val = atoi(wurds[3]);
                    euclidean_change_hits(e, val);
                }
                if (strncmp(wurds[2], "steps", 5) == 0)
                {
                    int val = atoi(wurds[3]);
                    euclidean_change_steps(e, val);
                }
            }
        }
        return true;
    }
    return false;
}
