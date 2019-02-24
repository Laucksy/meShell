#include "handlers.h"
#include "jobs.h"

void handleSIGCHLD(int sig) {
  // printf("SIGCHLD CALLED %d %d %d\n", sig, getpid(), tcgetpgrp(0));
  (void)sig;

  int status;
  waitpid(-1, &status, WNOHANG);
  alter_table_changed(tcgetpgrp(0), 0);

  return;
}
