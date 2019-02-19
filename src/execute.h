#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

struct job {
  int pid;
  char* status;
  int start;
  int end;
  char* cmd;
};
struct job job_table[5];

int execCommand(char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd);
void add_to_table(int pid, char *cmd);
void alter_table_ended(int pid, int status);

int handleBuiltin(char **tokens, int numTokens);
void handleCommand(char **tokens, int numTokens);
void handleEnv(char *input, int index);
