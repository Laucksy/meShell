#include "execute.h"

int job_table_len = 0;

int execCommand(char **tokens, int numTokens, int pipeIn, int pipeOut, int *fd) {
  // printf("Num tokens: %d\n", numTokens);
  // for (int i = 0; i < numTokens; i++) {
  //   printf("Token:%s\n", tokens[i]);
  // }

  int cmd_length = 0;
  for (int i = 0; i < numTokens; i++) {
    cmd_length += (int) strlen(tokens[i]);
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

void add_to_table(int pid, char *cmd) {
  time_t cur_time;
  time(&cur_time);
  char *status = "exec";

  char *test = malloc((strlen(cmd) + 1) * sizeof(char));
  for (int i = 0; i < (int) strlen(cmd); i++) test[i] = cmd[i];
  test[(int) strlen(cmd)] = '\0';

  // printf("Add to jobs table: %d ,%s, %d %s\n", pid, test, (int) cur_time, &(*status));
  job_table[job_table_len].pid = pid;
  job_table[job_table_len].status = status;
  job_table[job_table_len].start = cur_time;
  job_table[job_table_len].end = cur_time;
  job_table[job_table_len].cmd = test;
  job_table_len++;
}

void alter_table_ended(int pid, int ret) {
  // printf("ended %d %d\n", pid, ret);
  time_t cur_time;
  time(&cur_time);
  char *status = ret == 0 ? "ok" : "error";

  for (int i = 0; i < job_table_len; i++) {
    if (job_table[i].pid == pid) job_table[i].status = status;
  }
}

int handleBuiltin(char **tokens, int numTokens) {
  if (numTokens == 0) {
  } else if (strcmp(tokens[0], "exit") == 0) {
    exit(0);
  } else if (strcmp(tokens[0], "cd") == 0) {
    chdir(numTokens >= 2 ? tokens[1] : "~");
  } else if (strcmp(tokens[0], "jsum") == 0) {
    printf("PID\tStatus\tTime\tCMD\n");
    for (int i = 0; i < job_table_len; i++) {
      struct job j = job_table[i];
      printf("%d\t%s\t%d\t%s\n", (int) j.pid, j.status, (int) j.end - j.start, j.cmd);
    }
  } else {
    return 0;
  }
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

      pids[numPids] = execCommand(&tokens[i + 1], end - i - 1, pipeIn, pipeOut, fd);
      numPids++;
      end = i;

      close(pipeIn);
      pipeIn = fd[1];
      pipeOut = -1;
      close(pipeOut);
    }
  }

  int fd[2] = {pipeIn, pipeOut};
  int pid = execCommand(tokens, end, pipeIn, pipeOut, fd);
  if (!bg) {
    pids[numPids] = pid;
    numPids++;
  }
  if (pipeIn >= 0) close(pipeIn);

  for (int i = 0; i < numPids; i++) {
    if (pids[i]) {
      int status;
      waitpid(pids[i], &status, 0);
      alter_table_ended(pids[i], status);
    }
  }
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
