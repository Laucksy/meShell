#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define JOB_TABLE_MAX 5

struct job {
  int pid;
  char* status;
  int start;
  int end;
  char* cmd;
};
struct job job_table[JOB_TABLE_MAX];

int fg_pgid;

void add_to_table(int pid, char *cmd);
void alter_table_ended(int pid, int status);
void alter_table_changed(int pgid, int running);
void clean_table_and_exit();

void resume_job(char *arg, int bg);

void print_jsum();
void print_jobs();
