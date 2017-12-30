#include <stdio.h>
#include <string.h>

#include "defjams.h"
#include "euclidean.h"
#include "mixer.h"
#include "pattern_parser.h"
#include "sample_sequencer.h"
#include "sequencer_utils.h"

extern mixer *mixr;

enum pattern_token_type
{
    SQUARE_BRACKET_LEFT,
    SQUARE_BRACKET_RIGHT,
    CURLY_BRACKET_LEFT,
    CURLY_BRACKET_RIGHT,
    PAREN_BRACKET_LEFT,
    PAREN_BRACKET_RIGHT,
    VAR_NAME
} pattern_token_type;

#define MAX_PATTERN_CHAR_VAL 100
typedef struct pattern_token
{
    unsigned int type;
    char value[MAX_PATTERN_CHAR_VAL];
    int idx;
} pattern_token;

#define MAX_STACK_SIZE 30
int extract_tokens_from_pattern_wurds(pattern_token *tokens, int *token_idx,
                                       char *wurd)
{
    printf("TOKEN IDX:%d\n", *token_idx);
    printf("Looking at %s\n", wurd);

    int sq_bracket_balance = 0;

    char *c = wurd;
    while (*c)
    {
        printf("%c\n", *c);
        if (*c == '[')
        {
            sq_bracket_balance++;
            printf("SQ_LEFTBRACKET!\n");
        }
        else if (*c == ']')
        {
            sq_bracket_balance--;
            printf("SQ_RIGHTBRACKET!\n");
        }
        else
            printf("VAR!\n");
        c++;
    }

    return sq_bracket_balance;
}

typedef struct pg_child
{
    bool new_level;
    int level_idx;
} pg_child;

#define MAX_CHILDREN 20
typedef struct pattern_group
{
    int num_children;
    pg_child children[MAX_CHILDREN];
    int parent;
} pattern_group;

#define MAX_PATTERN 64
void work_out_positions(pattern_group pgroups[MAX_PATTERN],
                        int level,
                        int start_idx,
                        int pattern_len,
                        int ppositions[MAX_PATTERN],
                        int *numpositions)
{
    printf("Looking at Level:%d start_idx:%d pattern_len: %d\n", level, start_idx, pattern_len);
    int num_children = pgroups[level].num_children;
    int incr = pattern_len / num_children;
    pattern_len /= num_children;
    for (int i = 0; i < num_children; i++)
    {
        int child = pgroups[level].children[i].level_idx;
        int chidx = (i*incr) + start_idx;
        printf("CHILD:%d plays at pos%d\n", child, chidx);
        if (pgroups[level].children[i].new_level)
            printf("NEW LEVEL!\n");
        //work_out_positions(pgroups, child, chidx, incr, ppositions, numpositions);
    }
}

void parse_pattern(int num_wurds, char wurds[][SIZE_OF_WURD])
{
    pattern_token tokens[MAX_PATTERN];
    int token_idx = 0;
    printf("Got %d wurds\n", num_wurds);

    pattern_group pgroups[MAX_PATTERN] = {0};
    int current_pattern_group = 0;
    int num_pattern_groups = 0;

    for (int i = 0; i < num_wurds; i++)
    {
        if (strncmp(wurds[i], "[", 1) == 0)
        {
            printf("New group from %d\n", current_pattern_group);
            pgroups[++num_pattern_groups].parent = current_pattern_group;
            int num_children = ++pgroups[current_pattern_group].num_children;
            pgroups[current_pattern_group].children[num_children].level_idx = num_pattern_groups;
            pgroups[current_pattern_group].children[num_children].new_level = true;
            current_pattern_group = num_pattern_groups;
        }
        else if (strncmp(wurds[i], "]", 1) == 0)
        {
            current_pattern_group = pgroups[current_pattern_group].parent;
            printf("Right bracket - back down to %d\n", current_pattern_group);
        }
        else
        {
            printf("var %s\n", wurds[i]);
            pgroups[current_pattern_group].num_children++;
        }
    }

    printf("Num Groups:%d\n", num_pattern_groups);
    for (int i = 0; i <= num_pattern_groups; i++)
        printf("Group %d - parent is %d contains %d members\n", i, pgroups[i].parent, pgroups[i].num_children);

    int level = 0;
    int start_idx = 0;
    int pattern_len = PPBAR;
    int ppositions[MAX_PATTERN];
    int numpositions;
    work_out_positions(pgroups, level, start_idx, pattern_len, ppositions, &numpositions);
    //int sq_bracket_balance = 0;
    //for (int i = 0; i < num_wurds; i++)
    //{
    //    //if (mixer_is_valid_env_var(mixr, wurds[i]))
    //    //{
    //    //    printf("Valid ENV VAR! %s\n", wurds[i]);
    //    //    int sg_num;
    //    //    if (get_environment_val(wurds[i], &sg_num))
    //    //    {
    //    //        printf("CLEARING %s(%d)\n", wurds[i], sg_num);
    //    //        sample_sequencer *seq =
    //    //            (sample_sequencer *)mixr->sound_generators[sg_num];
    //    //        seq_clear_pattern(&seq->m_seq, 0);
    //    //    }
    //    //}
    //    //else
    //    //    printf("NAE Valid ENV VAR! %s\n", wurds[i]);
    //    sq_bracket_balance += extract_tokens_from_pattern_wurds(tokens, &token_idx, wurds[i]);
    //}

    //printf("\n");
    //if (sq_bracket_balance != 0)
    //    printf("NAH, MATE! brackets aren't balanced.\n");
    //else
    //    printf("Kosher, mate\n");

    //int rhythm = create_euclidean_rhythm(num_wurds, 16);
    //char rhythmbit[17];
    //char_binary_version_of_int(rhythm, rhythmbit);
    //printf("Pattern: %s\n", rhythmbit);

    //int wurd_idx = 0;
    //for (int i = 15; i >= 0; i--)
    //{
    //    if (rhythm & 1 << i)
    //    {
    //        int step = 15 - i;
    //        int sg_num;
    //        printf("YAR, HIT!\n");
    //        if (get_environment_val(wurds[wurd_idx], &sg_num))
    //        {
    //            printf("Playing %s(%d) at step:%d\n", wurds[wurd_idx], sg_num,
    //                   step);
    //            sample_sequencer *seq =
    //                (sample_sequencer *)mixr->sound_generators[sg_num];
    //            seq_add_hit(&seq->m_seq, 0, step);
    //        }
    //        wurd_idx++;
    //    }
    //}

    // for (int i = 0; i < token_idx; i++)
    //{
    //    printf("Tokens: %s\n", tokens[i].value);
    //}
    // 1. parse all wurds into tokens
    // 2. parse tokens into ordered groups
    // 3. parse groups into var separated patterns
    // 4. apply patterns to var/instruments
}
