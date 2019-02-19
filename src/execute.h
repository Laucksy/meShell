#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int execCommand(char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd);
int handleBuiltin(char **tokens, int numTokens);
void handleCommand(char **tokens, int numTokens);
void handleEnv(char *input, int index);
