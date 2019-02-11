#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  char *input = (char *) malloc(size + 1);
  size_t size = 1;
  
  while(strcmp(input, "exit") != 0) {
    printf("%s", title);
    input = (char *) realloc(input, size + 1);
    int length = getline(&input, &size, stdin);
    input[length - 1] = '\0';
  }

  return 0;
}
