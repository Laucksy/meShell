#include "handlers.h"
#include "execute.h"

void handleSIGINT(int sig) {
  printf("SIGINT CALLED %d\n", sig);
  // exit(0);
  return;
}

void handleSIGTSTP(int sig) {
  printf("SIGTSTP CALLED %d\n", sig);
  // exit(0);
  return;
}

void handleSIGCONT(int sig) {
  printf("SIGCONT CALLED %d\n", sig);
  // exit(0);
  return;
}

void handleSIGCHLD(int sig) {
  // printf("SIGCHLD CALLED %d\n", sig);

  int status;
  pid_t pid = wait(&status);

  while (pid != -1) {
    alter_table_ended(pid, status);
    pid = wait(&status);
  }

  return;
}
