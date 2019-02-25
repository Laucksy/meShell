#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#include "parse.h"
#include "execute.h"
#include "handlers.h"

int main() {
  printf("Welcome to        _____ __         ____\n");
  printf("   ____ ___  ___ / ___// /_  ___  / / /\n");
  printf("  / __ `__ \\/ _ \\\\__ \\/ __ \\/ _ \\/ / /\n");
  printf(" / / / / / /  __/__/ / / / /  __/ / /\n");
  printf("/_/ /_/ /_/\\___/____/_/ /_/\\___/_/_/\n");

  char *title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
  char *input;

  // Block signals in parent so the foreground child can handle them
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  // Use custom SIGCHILD to handle child state changes
  signal(SIGCHLD, handleSIGCHLD);

  using_history();
  input = readline(title);
  while(1) {
    // Tokenize input
    char input_copy[strlen(input) + 1];
    strcpy(input_copy, input);

    int numTokens = countTokens(input_copy);
    strcpy(input_copy, input);

    char *tokens[numTokens + 1];
    tokenize(input_copy, numTokens, tokens);

    int index = strcspn(input, "=");
    if (index < (int) strlen(input)) { // Check if trying to set environment variable
      handleEnv(input, index);
    } else if (!handleBuiltin(tokens, numTokens)) { // Check if trying to use builtin command
      handleCommand(tokens, numTokens); // Handle system command
    }
    add_history(input);
    free(input);

    title = getenv("lshprompt") ? getenv("lshprompt") : "lsh>";
    input = readline(title);
  }

  return 0;
}
