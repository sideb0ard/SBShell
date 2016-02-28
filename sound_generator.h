#ifndef SOUNDGEN_H
#define SOUNDGEN_H

#include "defjams.h"

// abstract class
//

typedef struct t_soundgen {
  // TODO : ENUM for type - i.e. OSC, FM or SAMPLE
  double (*gennext)(void *self);
  void (*status)(void *self, char *string);
  void (*setvol)(void *self, double val);
  double (*getvol)(void *self);
  //sound_generator_type type;
} SOUNDGEN;

#endif // SOUNDGEN_H