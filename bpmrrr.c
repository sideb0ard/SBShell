#define _POSIX_C_SOURCE 199309L

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "defjams.h"
#include "bpmrrr.h"

extern pthread_mutex_t bpm_lock;
extern pthread_cond_t bpm_cond;

bpmrrr *new_bpmrrr()
{
  bpmrrr *b = NULL;
  b = calloc(1, sizeof(bpmrrr));
  if (b == NULL) {
    fprintf(stderr, "Nae memory for a bpmrrr, mate.\n");
    return NULL;
  }

  b->bpm = DEFAULT_BPM;
  b->cur_tick = 0;
  //b->sleeptime = (60.0 / b->bpm / TICKS_PER_BEAT ) * 1000000000; 
  b->sleeptime = (60.0 / b->bpm / TICKS_PER_BEAT ) * 1000000000; 

  return b;
}

void bpm_change(bpmrrr *b, int bpm)
{
  if (bpm >= 60) { // my sleeptime calculation would break if this was under 60
    b->bpm = bpm;
    b->sleeptime = (60.0 / b->bpm / TICKS_PER_BEAT ) * 1000000000;
  }
}

void bpm_info(bpmrrr *b)
{
  printf(ANSI_COLOR_WHITE "BPM: %d // Current Tick: %d\n" ANSI_COLOR_RESET, b->bpm, b->cur_tick);
}

void *bpm_run(void *bp)
{
  bpmrrr *b = (bpmrrr*) bp;
  while (1)
  {
    pthread_mutex_lock(&bpm_lock);
    b->cur_tick++; // TICK TOCK!
    pthread_cond_broadcast(&bpm_cond);
    pthread_mutex_unlock(&bpm_lock);

    struct timespec ts;
    if ( b->sleeptime == 1000000000.000000 ) { // assuming won't go below 60bpm
      ts.tv_sec = 1;
      ts.tv_nsec = 0L;
    } else {
      ts.tv_sec = 0;
      ts.tv_nsec = b->sleeptime;
    }
    nanosleep(&ts, NULL);
  }
}
