#include "jobs.h"

int job_table_len = 0;
int job_table_base = 0; // Where the job table starts (> 0 once table overflows)
int job_table_ind = 0; // Next open position in table

int fg_pgid = 0; // Track foreground process group

/*
  Add new job to job table
  @param pid - pid of the new job
  @param cmd - command of the new job
 */
void add_to_table(int pid, char *cmd) {
  time_t cur_time;
  time(&cur_time);
  char *status = "exec";

  // Duplicate command
  char *test = malloc((strlen(cmd) + 1) * sizeof(char));
  for (int i = 0; i < (int) strlen(cmd); i++) test[i] = cmd[i];
  test[(int) strlen(cmd)] = '\0';

  // Initialize values in struct
  job_table[job_table_ind].pid = pid;
  job_table[job_table_ind].status = status;
  job_table[job_table_ind].start = cur_time;
  job_table[job_table_ind].end = cur_time;
  job_table[job_table_ind].cmd = test;

  // Handle overflow of jobs table
  job_table_len++;
  job_table_ind++;
  if (job_table_ind >= JOB_TABLE_MAX) {
    job_table_ind = 0;
  }
  if (job_table_len >= JOB_TABLE_MAX) {
    job_table_base = job_table_ind % JOB_TABLE_MAX;
  }
}

/*
  Update job table when job ends by any means
  @param pid - pid of the job
  @param ret - personal status code to indicate which status to show in job table
  @param return_status - system status code to display cause of termination if aborted
 */
void alter_table_ended(int pid, int ret, int return_status) {
  time_t cur_time;
  time(&cur_time);
  char *status = ret == 2 ? "abort" : (ret == 1 ? "error" : "ok");

  // Handle aborted jobs
  if (ret == 2 && return_status) {
    char *message = return_status == 2 ? "Interrupted" : (return_status == 9 ? "Killed" : "Other");
    printf("Job with pid %d ended with status %d (%s)\n", pid, return_status, message);
  }

  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    if (job_table[i].pid == pid) {
      job_table[i].status = status;
      job_table[i].end = cur_time;
      fg_pgid = 0;
      tcsetpgrp(0, getpgrp()); // Give foreground back to shell
    }
  }
}

/*
  Update job table when job changes state (foreground, background, stopped etc.)
  @param pgid - process group of the job
  @param running - personal status code to indicate which status to show in job table
 */
void alter_table_changed(int pgid, int running) {
  char *status = running == 1 ? "exec" : "stop";

  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    if (getpgid(job_table[i].pid) == pgid) {
      job_table[i].status = status;
    }
  }
}

/*
  Kill all unfinished processes and end the shell
 */
void clean_table_and_exit() {
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && (strcmp(j.status, "exec") == 0 || strcmp(j.status, "stop") == 0)) {
      kill(-1 * j.pid, SIGKILL);
    }
  }
  exit(0);
}

/*
  Move job to foreground or background
  @param arg - job id to move
  @param bg - whether or not to run in background
 */
void resume_job(char *arg, int bg) {
  int job_num = atoi(arg); // Convert to integer

  int num_found = 1;
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && (strcmp(j.status, "exec") == 0 || strcmp(j.status, "stop") == 0)) {
      if (num_found == job_num) { // Wait until right job num is found
        int pgid = getpgid(j.pid);

        // Restart and change status in job table
        kill(-1 * pgid, SIGCONT);
        alter_table_changed(pgid, 1);

        if (!bg) {
          fg_pgid = pgid;
          tcsetpgrp(0, fg_pgid); // Give foreground to child

          int status;
          waitpid(j.pid, &status, WUNTRACED);
          tcsetpgrp(0, getpgrp()); // Give foreground back to shell

          // Update job table depending on return status
          if (WIFEXITED(status) && status != 0) alter_table_ended(j.pid, 1, status); // error
          else if (WIFSIGNALED(status)) alter_table_ended(j.pid, 2, status); // abort
          else if (!WIFSTOPPED(status)) alter_table_ended(j.pid, 0, status); // success
        }
      }
      num_found++;
    }
  }
}

/*
  Print jsum table (all jobs and their stats)
 */
void print_jsum() {
  printf("PID\tStatus\tTime\tCMD\n");
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL) printf("%d\t%s\t%d sec\t%s\n", (int) j.pid, j.status, (int) j.end - j.start, j.cmd);
  }
  return;
}

/*
  Print job list (jobs that are running or stopped and their job id)
 */
void print_jobs() {
  int num_found = 1;
  for (int i = 0; i < JOB_TABLE_MAX; i++) {
    struct job j = job_table[(job_table_base + i) % JOB_TABLE_MAX];
    if (j.cmd != NULL && strcmp(j.status, "exec") == 0) printf("[%d]+\tRunning\t\t%s\n", num_found++, j.cmd);
    if (j.cmd != NULL && strcmp(j.status, "stop") == 0) printf("[%d]+\tStopped\t\t%s\n", num_found++, j.cmd);
  }
}
