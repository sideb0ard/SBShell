#include <stdio.h>
#include <stdlib.h>

#include "defjams.h"
#include "filter.h"
#include "utils.h"

void filter_setup(filter *f)
{
    f->m_q_control = 1.0; // Q is 1 to 10
    f->m_fc = FILTER_FC_DEFAULT;
    f->m_q = FILTER_Q_DEFAULT;
    f->m_fc_control = FILTER_FC_DEFAULT;

    f->m_fc_mod = 0.0;
    f->m_aux_control = 0.0;
    f->m_nlp = OFF;
    f->m_saturation = 1.0;

    f->g_modmatrix = NULL;
    f->m_mod_source_fc = DEST_NONE;
    f->m_mod_source_fc_control = DEST_NONE;
}

// void filter_set_fc_control(filter *f, double val)
//{
//    if (val > FILTER_FC_MIN && val < FILTER_FC_MAX) {
//        f->m_fc_control = val;
//    }
//}

void filter_set_fc_mod(filter *f, double val) { f->m_fc_mod = val; }

void filter_set_q_control(filter *f, double val) { f->m_q_control = val; }

void filter_update(filter *f)
{
    // filter_set_q_control(f, f->m_q_control);
    // printf("ORIG:FC: %f\n", f->m_fc);
    // printf("MY FC_CONTROL: %f\n", f->m_fc_control);
    f->m_fc = f->m_fc_control * pitch_shift_multiplier(f->m_fc_mod);

    if (f->m_fc > FILTER_FC_MAX)
        f->m_fc = FILTER_FC_MAX;
    if (f->m_fc < FILTER_FC_MIN)
        f->m_fc = FILTER_FC_MIN;
    // printf("NOW:FC: %f\n", f->m_fc);
    // printf("NOW MY FC_CONTROL: %f\n", f->m_fc_control);
}

void filter_reset(filter *f)
{
    (void)f; // noop
}
