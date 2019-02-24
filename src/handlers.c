#include "handlers.h"
#include "jobs.h"

/*
  Handle status updates from child processes
  Only called when child exits successfully
  @param sig - signal number sent to child
 */
void handleSIGCHLD(int sig) {
  // printf("SIGCHLD CALLED %d %d %d\n", sig, getpid(), tcgetpgrp(0));
  (void)sig;

  int status;
  waitpid(-1, &status, WNOHANG); // Get status of child
  alter_table_changed(tcgetpgrp(0), 0); // Update foreground process group with okay

  return;
}
