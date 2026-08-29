#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

void snooze(int duration) {
  int slept = sleep(duration);

  printf("slept for %d out of %d seconds\n", duration - slept, duration);
  return;
}

void handler(int signum)  {
  printf("interrupted!\n");
  return;
}
 
int main(int argc, char *argv[]) {
  if (signal(SIGINT, handler) == SIG_ERR) {
    err(EXIT_FAILURE, "failed to install signal handler\n");
  };

  snooze(strtol(argv[1], NULL, 10));

  return EXIT_SUCCESS;
}
