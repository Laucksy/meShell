#include "handlers.h"
#include "jobs.h"

void handleSIGINT(int sig) {
  printf("SIGINT CALLED %d\n", sig);
  if (fg_pgid > 0) kill(-1 * fg_pgid, SIGINT);
  return;
}

void handleSIGTSTP(int sig) {
  printf("SIGTSTP CALLED %d %d\n", sig, fg_pgid);
  if (fg_pgid > 0) kill(-1 * fg_pgid, SIGTSTP);
  alter_table_changed(fg_pgid, 0);
  return;
}

void handleSIGCONT(int sig) {
  printf("SIGCONT CALLED %d\n", sig);
  // exit(0);
  return;
}

void handleSIGCHLD(int sig) {
  // printf("SIGCHLD CALLED %d\n", sig);
  (void)sig;

  int status;
  pid_t pid = wait(&status);

  while (pid != -1) {
    alter_table_ended(pid, status);
    pid = wait(&status);
  }

  return;
}
