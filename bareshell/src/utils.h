#pragma once
#include "parser/command.h" 
#include <unistd.h> 

int resolve_program_path(const char *program_name, char *out, int out_size);
int execute_commands(struct Command *command_ptr);
int execute_single_command(struct Command *command_ptr, char *resolved_path, 
                             int *prev_pipe_read, int pipe_fd[2], 
                             int first_pid, int is_last);