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
  int pid = (int) waitpid(-1, &status, WNOHANG); // Get status of child

  while (pid >= 0) {
    if (pid == 0) alter_table_changed(tcgetpgrp(0), 0); // Update foreground process group with okay
    else {
      tcsetpgrp(0, getpgrp()); // Give foreground back to shell

      // Update job table depending on return status
      if (WIFEXITED(status) && status != 0) alter_table_ended(pid, 1, status); // error
      else if (WIFSIGNALED(status)) alter_table_ended(pid, 2, status); // abort
      else if (!WIFSTOPPED(status)) alter_table_ended(pid, 0, status);
    }

    if (pid > 0) pid = (int) waitpid(-1, &status, WNOHANG); // Get status of child
    else pid = -1; // Ensure pid of 0 doesn't run more than once
  }

  return;
}
