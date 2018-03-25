#pragma once

#include <stdbool.h>

#include "defjams.h"
#include "delayline.h"
#include "fx.h"
#include "lfo.h"

typedef enum
{
    TAP1,
    TAP2,
    PINGPONG,
    MAX_NUM_DELAY_MODE
} delay_mode;

typedef struct stereodelay
{
    fx m_fx; // API
    delayline m_left_delay, m_right_delay;
    double m_delay_time_ms;    // 0 - 2000
    double m_feedback_percent; // -100 - 100
    double m_delay_ratio;
    double m_wet_mix; // 0 - 100
    unsigned m_mode;
    double m_tap2_left_delay_time_ms;
    double m_tap2_right_delay_time_ms;

    lfo m_lfo1;
    bool lfo1_on;
    double m_lfo1_min;
    double m_lfo1_max;

    lfo m_lfo2;
    bool lfo2_on;
    double m_lfo2_min;
    double m_lfo2_max;
} stereodelay;

stereodelay *new_stereo_delay(double duration);

void stereo_delay_status(void *self, char *string);
double stereo_delay_process_wrapper(void *self, double input);

void stereo_delay_set_mode(stereodelay *d, unsigned mode);
void stereo_delay_set_delay_time_ms(stereodelay *d, double delay_ms);
void stereo_delay_set_feedback_percent(stereodelay *d, double feedback_percent);
void stereo_delay_set_delay_ratio(stereodelay *d, double delay_ratio);
void stereo_delay_set_wet_mix(stereodelay *d, double wet_mix);

void stereo_delay_prepare_for_play(stereodelay *d);
void stereo_delay_reset(stereodelay *d);
void stereo_delay_update(stereodelay *d);
bool stereo_delay_process_audio(stereodelay *d, double *input_left,
                                double *input_right, double *output_left,
                                double *output_right);
