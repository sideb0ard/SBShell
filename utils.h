typedef struct {
  char cmd[20];
  char params[20];
  int freq;
} sbmsg;

typedef enum {
  UP,
  DOWN
} direction;

void thrunner(sbmsg *msg);
void *timed_sig_start(void *arg);
void *fade_runrrr(void *arg);
//void startrrr(int sig_num);
void faderrr(int sig_num, direction d);
//void timed_sig_start(const char *sigtype, int freq, int freq2);
//void *algo_run(void *);
