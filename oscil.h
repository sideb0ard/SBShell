#ifndef OSCIL_H
#define OSCIL_H

#include <stdio.h>
#include "oscil.h"
#include "table.h"

typedef struct t_oscil OSCIL;
typedef double (*tickfunc) (OSCIL* osc);

typedef struct t_oscil
{
  double freq;
  double curphase;
  double incr;

  const GTABLE* gtable;
  double dtablen;

  tickfunc tick;
} OSCIL;

typedef double (*tickfunc) (OSCIL* tosc);

OSCIL* new_oscil(double freq, GTABLE *gt);
double tabtick(OSCIL* p_osc);
double tabitick(OSCIL* p_osc);

void status(OSCIL *p_osc, char *status_string);

#endif // OSCIL_H
