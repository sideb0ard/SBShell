#pragma once

#include "defjams.h"
#include "sequence_generator.h"

enum
{
    GARAGE,
    HIPHOP,
    HATS,
    NUM_MARKOV_STYLES
};

typedef struct markov
{
    sequence_generator sg;
    unsigned int markov_type;
} markov;

sequence_generator *new_markov(unsigned int type);
uint16_t markov_generate(void *self, void *data);
void markov_status(void *self, wchar_t *status_string);
void markov_event_notify(void *self, unsigned int event_type);
void markov_set_debug(void *self, bool b);