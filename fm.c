#include <stdlib.h>
#include <string.h>

#include "defjams.h"
#include "fm.h"
#include "table.h"

extern GTABLE *sine_table;

FM* new_fm(double modf, double carf)
{
  FM* fm;
  fm = (FM *) calloc(1, sizeof(FM));
  if (fm == NULL)
    return NULL;
  fm->mod_osc = new_oscil(modf, sine_table);
  fm->car_osc = new_oscil(carf, sine_table);

  fm->vol = 1.0;
  fm->mod_osc->voladj(fm->mod_osc, 0.7);
  fm->car_osc->voladj(fm->car_osc, 0.7);

  fm->sound_generator.gennext = &fm_gennext;
  fm->sound_generator.status = &fm_status;
  //fm->gen_next = &fm_gen_next;
  
  return fm;
}

double fm_gennext(void *self)
{
  FM *fm = (FM *) self;
  //float val = fm->cmod_osc->tick(fm->cmod_osc) * fm->fmod_osc->tick(fm->fmod_osc);
  double val = fm->car_osc->sound_generator.gennext(fm->car_osc);
  double mod_val = 100 * fm->mod_osc->sound_generator.gennext(fm->mod_osc);
  fm->car_osc->incradj(fm->car_osc, TABRAD * (fm->car_osc->freq + mod_val));

  return fm->vol * val;
}

void fm_status(void *self, char *status_string)
{
  FM *fm = (FM *) self;
  sprintf(status_string, "FM! modulator: %f %f // carrier: %f %f", fm->mod_osc->freq, fm->mod_osc->curphase, fm->car_osc->freq, fm->car_osc->curphase);
}

void mfm(FM *fm, char *osc, double val)
{
  if (!strcmp(osc, "mod")) {
    freqfunc(fm->mod_osc, val);
  } else if (!strcmp(osc, "car")) {
    freqfunc(fm->car_osc, val);
  } else {
    return;
  }
}
