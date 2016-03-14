#ifndef EFFECT_H
#define EFFECT_H

typedef enum {
  DELAY,
  LOWPASS,
  HIGHPASS
} effect_type;

typedef struct {
  double* buffer;
  int buf_p;
  int buf_length;
  double costh;
  double coef;
  effect_type type;
} EFFECT;

EFFECT* new_delay(double duration); 
EFFECT* new_freq_pass(double freq, effect_type pass_type); 

#endif
