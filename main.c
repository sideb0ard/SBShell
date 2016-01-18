#include <pthread.h>
#include <stdio.h>

#include "audioutils.h"
#include "cmdloop.h"
#include "defjams.h"
#include "mixer.h"

mixer *mixr;

int main()
{
  // PortAudio start me up!
  pa_setup();

  // run da mixer
  mixr = new_mixer();
  pthread_t mixrrun_th;
  if ( pthread_create (&mixrrun_th, NULL, mixer_run, (void*) mixr)) {
    fprintf(stderr, "Error running mixer_run thread\n");
    return 1;
  }
  pthread_detach(mixrrun_th);

  // interactive loop
  loopy();

  // all done, time to go home
  pa_teardown();

  return 0;
}
