#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

void execCommand(char **tokens, int numTokens, int *fd, int pipedInput);
void handleCommand(char *input);
void handleEnv(char *inupt, int index);
int countTokens(char *input);
void tokenize(char *input, int count, char **tokens);

extern char **environ;

int main(int argc, char *argv[]) {
  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  size_t size = 1;
  char *input;

  input = readline(title);
  while(strcmp(input, "exit") != 0) {
    int index = strcspn(input, "=");
    if (index < strlen(input)) {
      handleEnv(input, index);
    } else {
      handleCommand(input);
    }

    input = readline(title);
  }

  return 0;
}

void execCommand (char **tokens, int numTokens, int *fd, int pipedInput) {
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
    // printf("%d %d %d\n", pipedInput, fd[0], fd[1]);
    if (pipedInput >= 0) {
      if (pipedInput == 0) dup2(fd[1], 1);
      if (pipedInput == 1) dup2(fd[0], 0);

      close(fd[0]);
      close(fd[1]);
    }

    int ret = execvpe(tokens[0], tokens, environ);
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

  int fd[2];
  int index = 0;
  while (index < numTokens) {
    if (strcmp(tokens[index], "|") == 0) {
      tokens[index] = NULL;

      pipe(fd);
      execCommand(&tokens[index + 1], numTokens - index - 1, fd, 1);
      execCommand(tokens, index, fd, 0);
      close(fd[0]);
      close(fd[1]);

      while (wait(NULL) > 0);
      return;
    }
    index++;
  }

  execCommand(tokens, numTokens, fd, -1);
  wait(NULL);
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
