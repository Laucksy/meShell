#include <readline/readline.h>
#include <readline/history.h>

#include "parse.h"
#include "execute.h"

int main() {
  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  char *input;

  using_history();
  input = readline(title);
  while(1) {
    char input_copy[strlen(input) + 1];
    strcpy(input_copy, input);

    int numTokens = countTokens(input_copy);
    strcpy(input_copy, input);

    char *tokens[numTokens + 1];
    tokenize(input_copy, numTokens, tokens);

    int index = strcspn(input, "=");
    if (index < (int) strlen(input)) {
      handleEnv(input, index);
    } else if (!handleBuiltin(tokens, numTokens)) {
      handleCommand(tokens, numTokens);
    }
    add_history(input);
    free(input);

    input = readline(title);
  }

  return 0;
}
