#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

void execCommand(char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd);
void handleCommand(char *input);
void handleEnv(char *inupt, int index);
int countTokens(char *input);
void tokenize(char *input, int count, char **tokens);

int main() {
  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  char *input;

  using_history();
  input = readline(title);
  while(strcmp(input, "exit") != 0) {
    int index = strcspn(input, "=");
    if (index < (int) strlen(input)) {
      handleEnv(input, index);
    } else {
      handleCommand(input);
    }
    add_history(input);
    free(input);

    input = readline(title);
  }

  return 0;
}

void execCommand (char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd) {
  // printf("Num tokens: %d\n", numTokens);
  // for (int i = 0; i < numTokens; i++) {
  //   printf("Token:%s\n", tokens[i]);
  // }

  for (int i = 0; i < numTokens; i++) {
    if (tokens[i][0] == '$') {
      char *env_value = getenv(tokens[i] + 1) ? getenv(tokens[i] + 1) : "";
      tokens[i] = env_value;
    }
  }

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Unable to create fork.");
    exit(0);
  }
  if (pid == 0) {
    // printf("File Descriptors (%s) (%d): %d %d\n", tokens[0], getpid(), pipeIn, pipeOut);
    if (pipeIn >= 0) {
      dup2(pipeIn, 1);
      close(pipeIn);
    }
    if (pipeOut >= 0) {
      dup2(pipeOut, 0);
      close(pipeOut);
    }
    if (fd[0] >= 0) close(fd[0]);
    if (fd[1] >= 0) close(fd[1]);

    int ret = execvp(tokens[0], tokens);
    if (ret < 0) printf("lsh: %s: command not found\n", tokens[0]);
    exit(0);
  }
}

void handleCommand (char *input) {
  char input_copy[strlen(input) + 1];
  strcpy(input_copy, input);

  int numTokens = countTokens(input_copy);
  strcpy(input_copy, input);

  char *tokens[numTokens + 1];
  tokenize(input_copy, numTokens, tokens);

  int end = numTokens;
  int pipeIn = -1;
  int pipeOut = -1;
  for (int i = numTokens - 1; i > 0; i--) {
    if (strcmp(tokens[i], "|") == 0) {
      tokens[i] = NULL;

      int fd[2];
      pipe(fd);
      pipeOut = fd[0];

      execCommand(&tokens[i + 1], end - i - 1, pipeIn, pipeOut, fd);
      end = i;

      close(pipeIn);
      pipeIn = fd[1];
      pipeOut = -1;
      close(pipeOut);
    }
  }

  int fd[2] = {pipeIn, pipeOut};
  execCommand(tokens, end, pipeIn, pipeOut, fd);
  if (pipeIn >= 0) close(pipeIn);

  while (wait(NULL) > 0);
  return;
}

void handleEnv (char *input, int index) {
  char var_name[index + 1];
  memcpy(var_name, input, index);

  char value[strlen(input) - index];
  memcpy(value, &input[index + 1], strlen(input) - index);

  if (strlen(value) > 0) {
    setenv(var_name, value, 1);
  } else {
    unsetenv(var_name);
  }

  return;
}

int countTokens(char *input) {
  const char delimiter[2] = " ";
  int count = 0;

  char *token = strtok(input, delimiter);
  while( token != NULL ) {
    count++;
    token = strtok(NULL, delimiter);
  }
  return count;
}

void tokenize(char *input, int count, char **tokens) {
  const char delimiter[2] = " ";
  int index = 0;

  char *token = strtok(input, delimiter);
  while( token != NULL ) {
    tokens[index] = token;
    index++;

    token = strtok(NULL, delimiter);
  }
  tokens[count] = NULL;

  return;
}
