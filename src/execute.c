#include "execute.h"
#include "jobs.h"

int execCommand(char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd) {
  // printf("Num tokens: %d\n", numTokens);
  // for (int i = 0; i < numTokens; i++) {
  //   printf("Token:%s\n", tokens[i]);
  // }

  int cmd_length = 0;
  for (int i = 0; i < numTokens; i++) {
    if (tokens[i][0] == '$') {
      char *env_value = getenv(tokens[i] + 1) ? getenv(tokens[i] + 1) : "";
      tokens[i] = env_value;
    }
    cmd_length += (int) strlen(tokens[i]);
  }

  pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Unable to create fork.");
    exit(0);
  }
  if (pid == 0) {
    // printf("File Descriptors (%s) (%d): %d %d %d %d\n", tokens[0], getpid(), pipeIn, pipeOut, fd[0], fd[1]);
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

    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    int ret = execvp(tokens[0], tokens);
    if (ret < 0) {
      printf("lsh: %s: command not found\n", tokens[0]);
      exit(1);
    } else exit(0);
  }

  char cmd[cmd_length + numTokens];
  strcpy(cmd, "");
  for (int i = 0; i < numTokens; i++) {
    strcat(cmd, tokens[i]);
    strcat(cmd, " ");
  }
  cmd[cmd_length + numTokens - 1] = '\0';
  add_to_table(pid, cmd);

  return pid;
}

int handleBuiltin(char **tokens, int numTokens) {
  if (numTokens == 0) {}
  else if (strcmp(tokens[0], "exit") == 0) clean_table_and_exit();
  else if (strcmp(tokens[0], "cd") == 0) chdir(numTokens >= 2 ? tokens[1] : "~");
  else if (strcmp(tokens[0], "jsum") == 0) print_jsum();
  else if (strcmp(tokens[0], "jobs") == 0) print_jobs();
  else if (strcmp(tokens[0], "bg") == 0) resume_job(tokens[1], 1);
  else if (strcmp(tokens[0], "fg") == 0) resume_job(tokens[1], 0);
  else return 0;
  return 1;
}

void handleCommand(char **tokens, int numTokens) {
  int bg = 0;
  int pipeIn = -1;
  int pipeOut = -1;

  int pids[numTokens];
  int numPids = 0;
  if (strcmp(tokens[numTokens - 1], "&") == 0) {
    bg = 1;
    tokens[numTokens - 1] = NULL;
    numTokens--;
  }

  int end = numTokens;
  for (int i = numTokens - 1; i > 0; i--) {
    if (strcmp(tokens[i], "|") == 0) {
      tokens[i] = NULL;

      int fd[2];
      pipe(fd);
      pipeOut = fd[0];

      pids[numPids++] = execCommand(&tokens[i + 1], end - i - 1, pipeIn, pipeOut, fd);
      end = i;

      close(pipeIn);
      pipeIn = fd[1];
      pipeOut = -1;
      close(pipeOut);
    }
  }

  int fd[2] = {pipeIn, pipeOut};
  int pid = execCommand(tokens, end, pipeIn, pipeOut, fd);
  if (pipeIn >= 0) close(pipeIn);

  handleProcessControl(pids, numPids, pid, bg);
  return;
}

void handleEnv(char *input, int index) {
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

void handleProcessControl(int *pids, int numPids, int rootPid, int bg) {
  fg_pgid = bg ? 0 : rootPid;
  tcsetpgrp(0, fg_pgid);
  pids[numPids++] = rootPid;

  for (int i = numPids - 1; i >= 0; i--) {
    if (pids[i]) {
      setpgid(pids[i], rootPid);
    }
  }

  for (int i = 0; i < numPids; i++) {
    if (!bg && pids[i]) {
      int status;
      // printf("Wait: %d %d\n", pids[i], WUNTRACED);
      waitpid(pids[i], &status, WUNTRACED);
      tcsetpgrp(0, getpgrp());
      // printf("Got Wait: %d %d %d %d\n", status, WIFEXITED(status), WIFSIGNALED(status), WIFSTOPPED(status));

      // if (getpgid(pids[i]) == fg_pgid) fg_pgid = 0;

      if (WIFEXITED(status) && status != 0) alter_table_ended(pids[i], 1, status); // error
      else if (WIFSIGNALED(status)) alter_table_ended(pids[i], 2, status); // abort
      else if (!WIFSTOPPED(status)) alter_table_ended(pids[i], 0, status);
    }
  }

  return;
}
