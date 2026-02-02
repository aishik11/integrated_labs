#define _XOPEN_SOURCE 700
#include "utils.h"
#include "parser/command.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

Job job_table[MAX_JOBS];
int job_count = 0;

int resolve_program_path(const char *program_name, char *out, int out_size) {
  char *path_env = getenv("PATH");

  if (path_env == NULL) {
    return -1;
  }

  char *path_copy = malloc(strlen(path_env) + 1);
  strcpy(path_copy, path_env);
  // char* path_copy = string//strdup(path_env);
  if (path_copy == NULL) {
    perror("strdup");
    return -1;
  }

  snprintf(out, out_size, "%s/%s", ".", program_name);
  if (access(out, X_OK) == 0) {
    free(path_copy);
    return 0;
  } else {

    char *dir = strtok(path_copy, ":");
    while (dir != NULL) {
      snprintf(out, out_size, "%s/%s", dir, program_name);

      if (access(out, X_OK) == 0) {
        free(path_copy);
        return 0;
      }
      dir = strtok(NULL, ":");
    }
  }

  free(path_copy);
  return -1;
}

int execute_submit(struct Command *cmd) {
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Job table full\n");
        return -1;
    }
    if (cmd->args[1] == NULL) {
        fprintf(stderr, "Usage: submit <program.asm>\n");
        return -1;
    }

    char *asm_file = cmd->args[1];
    char bin_file[256];
    snprintf(bin_file, sizeof(bin_file), "%.*s.bin", (int)(strrchr(asm_file, '.') - asm_file), asm_file);
    
    char assembler_cmd[512];
    snprintf(assembler_cmd, sizeof(assembler_cmd), "bytecode_vm/build/assembler %s %s", asm_file, bin_file);
    if (system(assembler_cmd) != 0) {
        fprintf(stderr, "Assembly failed\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
      
        raise(SIGSTOP); 
        
        char *bvm_args[] = {"bvm", bin_file, NULL};
        
        execvp("bytecode_vm/build/bvm", bvm_args);  
        perror("execvp bvm");
        exit(1);
    } else if (pid > 0) {
        // Parent
        int status;
        waitpid(pid, &status, WUNTRACED); 
        
        Job new_job;
        new_job.id = job_count + 1; 
        new_job.pid = pid;
        new_job.filename = strdup(asm_file);
        new_job.binary_filename = strdup(bin_file);
        new_job.status = JOB_STATUS_SUBMITTED;
        
        job_table[job_count++] = new_job;
        printf("Submitted Program %s as Job %d (PID %d)\n", asm_file, new_job.id, pid);
    } else {
        perror("fork");
        return -1;
    }
    return 0;
}

int execute_run_job(struct Command *cmd) {
    if (cmd->args[1] == NULL) {
        fprintf(stderr, "Usage: run <job_id>\n");
        return -1;
    }
    int job_id = atoi(cmd->args[1]);
    int job_idx = -1;
    for (int i=0; i<job_count; i++) {
        if (job_table[i].id == job_id) {
            job_idx = i;
            break;
        }
    }
    
    if (job_idx == -1) {
        fprintf(stderr, "Job %d not found\n", job_id);
        return -1;
    }

    pid_t pid = job_table[job_idx].pid;
    printf("Resuming Job %d (PID %d)...\n", job_id, pid);
    job_table[job_idx].status = JOB_STATUS_RUNNING;

    
    if (kill(pid, SIGCONT) == -1) {
        perror("kill SIGCONT");
        return -1;
    }

    int status;

    
    if (tcsetpgrp(STDIN_FILENO, pid) == -1) {
         // perror("tcsetpgrp"); // might fail if session mismatch, ignore for now
    }
    
    waitpid(pid, &status, WUNTRACED);
    
    if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) {
        // perror("tcsetpgrp back");
    }

    if (WIFEXITED(status)) {
        printf("Job %d finished with exit code %d\n", job_id, WEXITSTATUS(status));
        job_table[job_idx].status = JOB_STATUS_FINISHED;
    } else if (WIFSIGNALED(status)) {
        printf("Job %d terminated by signal %d\n", job_id, WTERMSIG(status));
        job_table[job_idx].status = JOB_STATUS_TERMINATED;
    } else if (WIFSTOPPED(status)) {
        printf("Job %d stopped by signal %d\n", job_id, WSTOPSIG(status));
        // stays running/stopped?
    }
    
    return 0;
}

int execute_debug_job(struct Command *cmd) {
  
    
    if (cmd->args[1] == NULL) {
        fprintf(stderr, "Usage: debug <job_id>\n");
        return -1;
    }
    int job_id = atoi(cmd->args[1]);
     int job_idx = -1;
    for (int i=0; i<job_count; i++) {
        if (job_table[i].id == job_id) {
            job_idx = i;
            break;
        }
    }
    if (job_idx == -1) {
        fprintf(stderr, "Job %d not found\n", job_id);
        return -1;
    }
    
    pid_t pid = job_table[job_idx].pid;
    
    // Send signal to enable debug mode in VM
    // We assume VM handles SIGUSR1 to set debug_mode = true
    kill(pid, SIGUSR1); 
    
    // Now resume
    return execute_run_job(cmd);
}


int execute_commands(struct Command *command_ptr) {

  int prev_pipe_read = STDIN_FILENO;
  int pipe_fd[2];
  pid_t first_pid = 0;
  int is_background = 0;
  is_background = command_ptr->is_background;

  // Intercept submit / run / debug
  if (command_ptr->next_command == NULL) {
      if (strcmp(command_ptr->prog_name, "submit") == 0) {
          return execute_submit(command_ptr);
      }
      if (strcmp(command_ptr->prog_name, "run") == 0) {
          return execute_run_job(command_ptr);
      }
      if (strcmp(command_ptr->prog_name, "debug") == 0) {
          return execute_debug_job(command_ptr);
      }
  }

  // handling exit
  if (command_ptr->next_command == NULL &&
      strcmp(command_ptr->prog_name, "exit") == 0) {

    int exit_code = 0;
    exit(exit_code);
  }

  // handling cd
  if (command_ptr->next_command == NULL &&
      strcmp(command_ptr->prog_name, "cd") == 0) {

    const char *target_dir;

    if (command_ptr->args[1] == NULL) {
      target_dir = getenv("HOME");
      if (target_dir == NULL) {
        fprintf(stderr, "cd: HOME not set\n");
        return -1;
      }
    } else {
      target_dir = command_ptr->args[1];
    }

    if (chdir(target_dir) != 0) {
      perror("cd");
      return -1;
    }
    return 0;
  }

  while (command_ptr != NULL) {
    int is_last = (command_ptr->next_command == NULL ||
                   strcmp(command_ptr->next_command->prog_name, "cd") == 0 ||
                   strcmp(command_ptr->next_command->prog_name, "exit") == 0);

    if (strcmp(command_ptr->prog_name, "cd") == 0 ||
        strcmp(command_ptr->prog_name, "exit") == 0) {
      command_ptr = command_ptr->next_command;
      continue;
    }
    
    // Check for submit/run in pipes? (Unlikely requirement for now)

    if (!is_last) {
      if (pipe(pipe_fd) == -1) {
        perror("pipe error");
        return -1;
      }
    }

    char buffer[1024];
    resolve_program_path(command_ptr->prog_name, buffer, sizeof(buffer));

    pid_t pid = execute_single_command(command_ptr, buffer, &prev_pipe_read,
                                       pipe_fd, first_pid, is_last);
    if (first_pid == 0) {
      first_pid = pid;
    }

    command_ptr = command_ptr->next_command;
  }

  if (first_pid > 0) {
    // tcsetpgrp(STDIN_FILENO, first_pid);
    if (!is_background) {

      signal(SIGTTOU, SIG_IGN);

          if (isatty(STDIN_FILENO)) {
              if (tcsetpgrp(STDIN_FILENO, first_pid) == -1) {
                  perror("tcsetpgrp child");
              }
          }
      int status;
      while (waitpid(-first_pid, &status, WUNTRACED) > 0) {
        if (WIFSTOPPED(status)) {
        }
      }

          if (isatty(STDIN_FILENO)) {
              if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) {
                  perror("tcsetpgrp parent");
              }
          }
      signal(SIGTTOU, SIG_DFL);
    }

    // tcsetpgrp(STDIN_FILENO, getpid());
  }

  return 0;
}

int execute_single_command(struct Command *command_ptr, char *resolved_path,
                           int *prev_pipe_read, int pipe_fd[2], pid_t first_pid,
                           int is_last) {
  pid_t pid = fork();
  int background = command_ptr->is_background;
  if (pid == 0) {
    // child

    pid_t group_id = (first_pid == 0) ? getpid() : first_pid;
    setpgid(0, group_id);

    if (background == 0) {
  
      // tcsetpgrp(STDIN_FILENO, group_id);
    }

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);


    if (*prev_pipe_read != STDIN_FILENO) {
      dup2(*prev_pipe_read, STDIN_FILENO);
      close(*prev_pipe_read);
    }

    if (!is_last) {
      close(pipe_fd[0]);
      dup2(pipe_fd[1], STDOUT_FILENO);
      close(pipe_fd[1]);
    }

    // handle < in file
    if (command_ptr->in_file != NULL) {
      int in_fd = open(command_ptr->in_file, O_RDONLY);
      if (in_fd == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
      }
      dup2(in_fd, STDIN_FILENO);
      close(in_fd);
    }

    // handle > out file
    if (command_ptr->out_file != NULL) {
      int out_fd =
          open(command_ptr->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (out_fd == -1) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
      }
      dup2(out_fd, STDOUT_FILENO);
      close(out_fd);
    }

    if (execve(resolved_path, command_ptr->args, NULL) == -1) {
      perror("");
      exit(EXIT_FAILURE);
    }
  } else if (pid < 0) {
    perror("fork error");
    return -1;
  }

  setpgid(pid, (first_pid == 0) ? pid : first_pid);

  if (*prev_pipe_read != STDIN_FILENO) {
    close(*prev_pipe_read);
  }

  if (!is_last) {
    *prev_pipe_read = pipe_fd[0];
    close(pipe_fd[1]);
  }

  return pid;
}
