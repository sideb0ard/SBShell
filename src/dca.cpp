#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "dca.h"
#include "defjams.h"
#include "utils.h"

#include <iostream>

extern const char *s_dest_enum_to_name[];
dca *new_dca()
{
    dca *d = (dca *)calloc(1, sizeof(dca));
    if (!d)
        return NULL;

    dca_initialize(d);

    return d;
}

void dca_initialize(dca *d)
{
    d->m_amplitude_control = 1.0;
    d->m_amp_mod_db = 0.0;
    d->m_gain = 1.0;
    d->m_amplitude_db = 0.0;
    d->m_eg_mod = 1.0;
    d->m_pan_control = 0.0;
    d->m_pan_mod = 0.0;
    d->m_midi_velocity = 127;

    d->m_v_modmatrix = nullptr;
    d->m_global_dca_params = nullptr;

    d->m_mod_source_eg = DEST_NONE;
    d->m_mod_source_amp_db = DEST_NONE;
    d->m_mod_source_velocity = DEST_NONE;
    d->m_mod_source_pan = DEST_NONE;
}

void dca_set_midi_velocity(dca *self, unsigned int vel)
{
    self->m_midi_velocity = vel;
}

void dca_set_pan_control(dca *self, double pan) { self->m_pan_control = pan; }

void dca_reset(dca *self)
{
    self->m_eg_mod = 1.0;
    self->m_amp_mod_db = 0.0;
}

void dca_set_amplitude_db(dca *self, double amp)
{
    self->m_amplitude_db = amp;
    self->m_amplitude_control = pow((double)10.0, amp / (double)20.0);
}

void dca_set_amp_mod_db(dca *self, double mod) { self->m_amp_mod_db = mod; }

void dca_set_eg_mod(dca *self, double mod) { self->m_eg_mod = mod; }

void dca_set_pan_mod(dca *self, double mod) { self->m_pan_mod = mod; }

void dca_update(dca *self)
{
    if (self->m_global_dca_params)
    {
        dca_set_amplitude_db(self, self->m_global_dca_params->amplitude_db);
        self->m_pan_control = self->m_global_dca_params->pan_control;
    }

    if (self->m_v_modmatrix)
    {
        // printf("Yup yup, got modmatrix\n");
        if (self->m_mod_source_eg != DEST_NONE)
        {
            self->m_eg_mod =
                self->m_v_modmatrix->m_destinations[self->m_mod_source_eg];
        }
        if (self->m_mod_source_amp_db != DEST_NONE)
            self->m_amp_mod_db =
                self->m_v_modmatrix->m_destinations[self->m_mod_source_amp_db];

        if (self->m_mod_source_velocity != DEST_NONE)
        {
            self->m_midi_velocity =
                self->m_v_modmatrix
                    ->m_destinations[self->m_mod_source_velocity];
            // std::cout << "GOT VELoCITY! " << self->m_mod_source_velocity << "
            // "
            //          << self->m_midi_velocity << std::endl;
        }

        if (self->m_mod_source_pan != DEST_NONE)
            self->m_pan_mod =
                self->m_v_modmatrix->m_destinations[self->m_mod_source_pan];
    }

    if (self->m_eg_mod >= 0)
        self->m_gain = self->m_eg_mod;
    else
        self->m_gain = self->m_eg_mod + 1.0;

    self->m_gain *= pow(10.0, self->m_amp_mod_db / (double)20.0);
    self->m_gain *= mma_midi_to_atten(self->m_midi_velocity);
}

void dca_gennext(dca *self, double left_input, double right_input,
                 double *left_output, double *right_output)
{
    double pan_total = self->m_pan_control + self->m_pan_mod;

    pan_total = fmin(pan_total, 1.0);
    pan_total = fmax(pan_total, -1.0);
    double pan_left = 0.707;
    double pan_right = 0.707;
    calculate_pan_values(pan_total, &pan_left, &pan_right);

    *left_output =
        pan_left * self->m_amplitude_control * left_input * self->m_gain;
    *right_output =
        pan_right * self->m_amplitude_control * right_input * self->m_gain;
}

void dca_init_global_parameters(dca *self, global_dca_params *params)
{
    self->m_global_dca_params = params;
    params->amplitude_db = self->m_amplitude_db;
    params->pan_control = self->m_pan_control;
}
