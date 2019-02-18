#include "parse.h"

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
