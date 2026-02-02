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

int execute_commands(struct Command *command_ptr) {

  int prev_pipe_read = STDIN_FILENO;
  int pipe_fd[2];
  pid_t first_pid = 0;
  int is_background = 0;
  is_background = command_ptr->is_background;

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
