#include "jobs.h"

int job_table_len = 0;
int job_table_base = 0;
int job_table_ind = 0;

int fg_pgid = 0;

void add_to_table(int pid, char *cmd) {
  time_t cur_time;
  time(&cur_time);
  char *status = "exec";

  char *test = malloc((strlen(cmd) + 1) * sizeof(char));
  for (int i = 0; i < (int) strlen(cmd); i++) test[i] = cmd[i];
  test[(int) strlen(cmd)] = '\0';

  // printf("Add to jobs table: %d ,%s, %d %s\n", pid, test, (int) cur_time, &(*status));
  job_table[job_table_ind].pid = pid;
  job_table[job_table_ind].status = status;
  job_table[job_table_ind].start = cur_time;
  job_table[job_table_ind].end = cur_time;
  job_table[job_table_ind].cmd = test;

  job_table_len++;
  job_table_ind++;
  if (job_table_ind >= JOB_TABLE_MAX) {
    job_table_ind = 0;
  }
  if (job_table_len >= JOB_TABLE_MAX) {
    job_table_base = job_table_ind % JOB_TABLE_MAX;
  }
}

void alter_table_ended(int pid, int ret, int return_status) {
  // printf("ended %d %d\n", pid, ret);
  time_t cur_time;
  time(&cur_time);
  char *status = ret == 2 ? "abort" : (ret == 1 ? "error" : "ok");

  if (ret == 2 && return_status) {
    char *message = return_status == 2 ? "Interrupted" : (return_status == 9 ? "Killed" : "Other");
    printf("Job with pid %d ended with status %d (%s)\n", pid, return_status, message);
  }

  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    if (job_table[i].pid == pid) {
      // printf("End jobs table: %d %d %s\n", pid, (int) cur_time, &(*status));
      job_table[i].status = status;
      job_table[i].end = cur_time;
      fg_pgid = 0;
      // printf("PID %d %d %d", getpid(), getpgid(getpid()), getpgrp());
      tcsetpgrp(0, getpgrp());
    }
  }
}

void alter_table_changed(int pgid, int running) {
  char *status = running == 1 ? "exec" : "stop";

  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    if (getpgid(job_table[i].pid) == pgid) {
      job_table[i].status = status;
    }
  }
}

void clean_table_and_exit() {
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && (strcmp(j.status, "exec") == 0 || strcmp(j.status, "stop") == 0)) {
      printf("Kill PID: %d", j.pid);
      kill(-1 * j.pid, SIGKILL);
    }
  }
  exit(0);
}

void resume_job(char *arg, int bg) {
  int job_num = atoi(arg);

  int num_found = 1;
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && (strcmp(j.status, "exec") == 0 || strcmp(j.status, "stop") == 0)) {
      if (num_found == job_num) {
        int pgid = getpgid(j.pid);

        kill(-1 * pgid, SIGCONT);
        alter_table_changed(pgid, 1);

        if (!bg) {
          fg_pgid = pgid;
          tcsetpgrp(0, fg_pgid);

          int status;
          waitpid(j.pid, &status, WUNTRACED);
          tcsetpgrp(0, getpgrp());
          if (WIFEXITED(status) && status != 0) alter_table_ended(j.pid, 1, status); // error
          else if (WIFSIGNALED(status)) alter_table_ended(j.pid, 2, status); // abort
          else if (!WIFSTOPPED(status)) alter_table_ended(j.pid, 0, status);
        }
      }
      num_found++;
    }
  }
}

void print_jsum() {
  printf("PID\tStatus\tTime\tCMD\n");
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL) printf("%d\t%s\t%d sec\t%s\n", (int) j.pid, j.status, (int) j.end - j.start, j.cmd);
  }
  return;
}

void print_jobs() {
  int num_found = 1;
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && strcmp(j.status, "exec") == 0) printf("[%d]+\tRunning\t\t%s\n", num_found++, j.cmd);
    if (j.cmd != NULL && strcmp(j.status, "stop") == 0) printf("[%d]+\tStopped\t\t%s\n", num_found++, j.cmd);
  }
}
