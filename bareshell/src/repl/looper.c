#define _POSIX_C_SOURCE 200809L
#include "looper.h"
#include "../io/input.h"
#include "../parser/command.h"
#include "../utils.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static volatile sig_atomic_t g_sigint_received = 0;

void handle_sigint(int sig) {
    (void)sig;
    g_sigint_received = 1;
}

enum repl_error looper() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0; // Ensure system calls are interrupted (no SA_RESTART)
  sigaction(SIGINT, &sa, NULL);

  char *buffer = NULL;
  buffer = malloc(MAX_BUFFER_SIZE);
  if (!buffer)
    return REPL_MALLOC_FAIL;
  struct Command *command = NULL;
  command = malloc(sizeof(struct Command));
  if (!command) {
    free(buffer);
    return REPL_MALLOC_FAIL;
  }
  enum repl_error exit_code = REPL_SUCCESS;

  while (exit_code == REPL_SUCCESS) {
    g_sigint_received = 0;
    memset(buffer, 0, MAX_BUFFER_SIZE); // Clear buffer before new input
    multiline_input(buffer, MAX_BUFFER_SIZE, stdin);

    if (g_sigint_received) {
        free(buffer);
        free(command);
        return REPL_SIGINT;
    }

    char *parser_input_buffer = buffer;
    if (feof(stdin)) { 
        parser_input_buffer = NULL; 
    }

    enum parse_error pe = parse_to_command(parser_input_buffer, command);
    switch (pe) {
    case PARSER_EOF:
      exit_code = REPL_SUCCESS; 
      break; 
    case PARSER_TOKEN_DONE:
      printf("PARSER_TOKEN_DONE\n");
      exit_code = REPL_UNEXPECTED_MODULE_FAIL;
      break;
    case PARSER_SUCCESS:
      break;
    case PARSER_UNPAIRED_QUOTES:
      printf("You have unpaired quotes in the command\n");
      break;
    case PARSER_MALLOC_FAIL:
      printf("FATAL: internal error\n");
      exit_code = REPL_MALLOC_FAIL;
      break;
    case PARSER_NO_COMMAND:
    case PARSER_MISSING_COMMAND:
      printf("Please enter a command\n");
      break;
    case PARSER_NO_IN_FILE:
    case PARSER_NO_OUT_FILE:
      printf("Please specify the redirection file\n");
      break;
    case PARSER_DUPLICATE_IN_REDIR:
    case PARSER_DUPLICATE_OUT_REDIR:
      printf("Multiple redirections found\n");
      break;
    case PARSER_SYNTAX_ERROR:
      printf("Invalid Syntax, please try again\n");
      break;
    }
    if (exit_code != REPL_SUCCESS)
      break;
    
    if (pe == PARSER_EOF) {
        break;
    }

    if (pe == PARSER_SUCCESS) {

      execute_commands(command);
    }
  }
  free(buffer);
  free(command);
  return exit_code;
}
