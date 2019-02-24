#include "parse.h"

/*
  First pass through command to allow exact array size on initializiation
  @param input - command input by user
  @return number of tokens in the line
 */
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

/*
  Split a command into an array of tokens
  @param input - command input by user
  @param count - number of tokens to be expected
  @param tokens - array to insert tokens in
 */
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
