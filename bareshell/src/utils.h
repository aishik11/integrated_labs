#pragma once
#include "parser/command.h" 
#include "jobs.h"
#include <unistd.h>

#define MAX_JOBS 10
extern Job job_table[MAX_JOBS];
extern int job_count;

int resolve_program_path(const char *program_name, char *out, int out_size);
int execute_submit(struct Command *cmd);
int execute_run_job(struct Command *cmd);
int execute_debug_job(struct Command *cmd);
int execute_commands(struct Command *command_ptr);
int execute_single_command(struct Command *command_ptr, char *resolved_path,
                             int *prev_pipe_read, int pipe_fd[2],
                             int first_pid, int is_last);