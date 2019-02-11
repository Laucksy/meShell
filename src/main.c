#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int countTokens(char *input);
void tokenize(char *input, int count, char **tokens);

int main(int argc, char *argv[]) {
  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  size_t size = 1;
  char *input = (char *) malloc(size + 1);

  while(strcmp(input, "exit") != 0) {
    printf("%s", title);
    input = (char *) realloc(input, size + 1);
    int length = getline(&input, &size, stdin);
    input[length - 1] = '\0';

    int index = strcspn(input, "=");
    if (index < strlen(input)) {
      char var_name[index + 1];
      memcpy(var_name, input, index);

      char value[strlen(input) - index];
      memcpy(value, &input[index + 1], strlen(input) - index);

      if (strlen(value) > 0) {
        setenv(var_name, value, 1);
      } else {
        unsetenv(var_name);
      }
    } else {
      if (strcmp(input, "$test") == 0) {
        char *value = getenv("test") ? getenv("test") : "";
        // printf("%s\n", value);
      }

      char input_copy[strlen(input) + 1];
      strcpy(input_copy, input);

      int numTokens = countTokens(input_copy);
      strcpy(input_copy, input);

      char *tokens[numTokens];
      tokenize(input_copy, numTokens, tokens);

      for (int i = 0; i < numTokens; i++) {
        printf("token %d %s\n", i, tokens[i]);
      }
    }
  }

  return 0;
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
  return;
}
