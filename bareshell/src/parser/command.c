#include "command.h"
#include <stdlib.h>
#include <string.h>

const char *parse_error_strings[] = {
    "PARSER_TOKEN_DONE",         "PARSER_SUCCESS",
    "PARSER_UNPAIRED_QUOTES",    "PARSER_MALLOC_FAIL",
    "PARSER_NO_COMMAND",         "PARSER_MISSING_COMMAND",
    "PARSER_NO_IN_FILE",         "PARSER_NO_OUT_FILE",
    "PARSER_DUPLICATE_IN_REDIR", "PARSER_DUPLICATE_OUT_REDIR",
    "PARSER_SYNTAX_ERROR",       "PARSER_EOF"};

int is_space(char c) { return (c == ' ' || c == '\t' || c == '\n'); }

char is_quote(char c) {
  if (c == '\"' || c == '"') {
    return c;
  } else {
    return '\0';
  }
}

int is_redir(char c) { return c == '>' || c == '<' || c == '|' || c == '&'; }

// function to extract a single token
enum parse_error get_token(char **buffer, char **token) {
  // Initialize token
  *token = NULL;

  // skip initial spaces
  while (is_space(**buffer))
    (*buffer)++;

  if (**buffer == '\0')
    return PARSER_TOKEN_DONE; // no more tokens

  // start of the token
  char *start = *buffer;

  // case 1 handling quoted token
  char quote = is_quote(**buffer);
  if (quote != '\0') {
    (*buffer)++;
    // Loop to find the matching quote
    while (1) {
      // If the buffer ends before the quote is matched
      if (**buffer == '\0')
        return PARSER_UNPAIRED_QUOTES;
      if (is_quote(**buffer) == quote && *(*buffer - 1) != '\\') {
        // Found the other quote
        // So copy the contents to token with zero ending
        size_t len = *buffer - start + 1;
        *token = malloc((len + 1) * sizeof(char));
        if (!*token)
          return PARSER_MALLOC_FAIL;
        strncpy(*token, start, len);
        (*token)[len] = '\0';
        (*buffer)++;
        return PARSER_SUCCESS;
      }
      (*buffer)++;
    }

    // Otherwise handle single char tokens
  } else if (is_redir(*start)) {
    *token = malloc(2 * sizeof(char));
    if (!*token)
      return PARSER_MALLOC_FAIL;
    *token = strncpy(*token, start, 1);
    (*token)[1] = '\0';
    (*buffer)++;
    return PARSER_SUCCESS;

  } else {
    // Finally otherwise, until a stop is reached the entirety is a token
    char *end = start;
    while (*end && !is_quote(*end) && !is_space(*end) && !is_redir(*end))
      end++;
    size_t len = end - start;
    *token = malloc((len + 1) * sizeof(char));
    if (!*token)
      return PARSER_MALLOC_FAIL;
    strncpy(*token, start, len);
    (*token)[len] = '\0';
    *buffer = end;
    return PARSER_SUCCESS;
  }
}

enum parse_error parse_command_helper(char *tokens[], int *token_idx_ptr,
                                      struct Command *command_ptr) {
  int current_token_idx = *token_idx_ptr;

  if (!tokens[current_token_idx] ||
      strcmp(tokens[current_token_idx], ">") == 0 ||
      strcmp(tokens[current_token_idx], "<") == 0 ||
      strcmp(tokens[current_token_idx], "|") == 0 ||
      strcmp(tokens[current_token_idx], "&") == 0) {
    return PARSER_MISSING_COMMAND;
  }

  char **args_array = malloc(sizeof(char *) * MAX_TOKEN_COUNT);
  if (!args_array) {
    return PARSER_MALLOC_FAIL;
  }
  int arg_idx = 0;
  command_ptr->args = args_array;

  args_array[arg_idx++] = tokens[current_token_idx];
  command_ptr->prog_name = args_array[0];
  current_token_idx++;
  args_array[arg_idx] = NULL;

  int out_done = 0;
  int in_done = 0;
  while (tokens[current_token_idx] &&
         strcmp(tokens[current_token_idx], "|") != 0) {
    if (strcmp(tokens[current_token_idx], ">") == 0) {
      if (out_done) {
        free(args_array);
        return PARSER_DUPLICATE_OUT_REDIR;
      }
      current_token_idx++;
      if (!tokens[current_token_idx] ||
          strcmp(tokens[current_token_idx], "<") == 0 ||
          strcmp(tokens[current_token_idx], ">") == 0 ||
          strcmp(tokens[current_token_idx], "|") == 0) {
        free(args_array);
        return PARSER_NO_OUT_FILE;
      }
      command_ptr->out_file = tokens[current_token_idx];
      current_token_idx++;
      out_done = 1;
    } else if (strcmp(tokens[current_token_idx], "<") == 0) {
      if (in_done) {
        free(args_array);
        return PARSER_DUPLICATE_IN_REDIR;
      }
      current_token_idx++;
      if (!tokens[current_token_idx] ||
          strcmp(tokens[current_token_idx], "<") == 0 ||
          strcmp(tokens[current_token_idx], ">") == 0 ||
          strcmp(tokens[current_token_idx], "|") == 0) {
        free(args_array);
        return PARSER_NO_IN_FILE;
      }
      command_ptr->in_file = tokens[current_token_idx];
      current_token_idx++;
      in_done = 1;
    } else if (strcmp(tokens[current_token_idx], "&") == 0) {
      break;
    } else {
      args_array[arg_idx++] = tokens[current_token_idx];
      args_array[arg_idx] = NULL;
      current_token_idx++;
    }
  }

  if (tokens[current_token_idx] &&
      strcmp(tokens[current_token_idx], "|") == 0) {
    current_token_idx++;
    struct Command *next_cmd = malloc(sizeof(struct Command));
    if (!next_cmd) {
      free(args_array);
      return PARSER_MALLOC_FAIL;
    }
    next_cmd->next_command = NULL;
    next_cmd->args = NULL;
    next_cmd->in_file = NULL;
    next_cmd->out_file = NULL;
    next_cmd->prog_name = NULL;
    next_cmd->is_background = 0;

    command_ptr->next_command = next_cmd;
    enum parse_error err =
        parse_command_helper(tokens, &current_token_idx, next_cmd);
    if (err != PARSER_SUCCESS) {
      free(args_array);
      return err;
    }
  }

  *token_idx_ptr = current_token_idx;
  return PARSER_SUCCESS;
}

enum parse_error parse_to_command(char *input_buffer,
                                  struct Command *command_ptr) {
  if (!input_buffer) {
    return PARSER_EOF;
  }
  // init Command struct
  command_ptr->next_command = NULL;
  command_ptr->args = NULL;
  command_ptr->in_file = NULL;
  command_ptr->out_file = NULL;
  command_ptr->prog_name = NULL;
  command_ptr->is_background = 0;

  // Get all the tokens
  char *tokens[MAX_TOKEN_COUNT] = {0};
  int token_count = 0;
  char *buffer = input_buffer;

  // collect tokens for all commands
  char *token;
  while (token_count < MAX_TOKEN_COUNT) {
    enum parse_error err = get_token(&buffer, &token);
    if (err != PARSER_SUCCESS) {
      if (err == PARSER_TOKEN_DONE)
        break;
      return err;
    }
    tokens[token_count++] = token;
  }

  if (token_count == 0) {
    return PARSER_NO_COMMAND;
  }

  int token_idx = 0;
  enum parse_error err = parse_command_helper(tokens, &token_idx, command_ptr);
  if (err != PARSER_SUCCESS) {
    return err;
  }

  if (tokens[token_idx] && strcmp(tokens[token_idx], "&") == 0) {
    struct Command *cmd = command_ptr;
    while (cmd) {
      cmd->is_background = 1;
      cmd = cmd->next_command;
    }
    token_idx++;
  }

  if (tokens[token_idx]) {
    return PARSER_SYNTAX_ERROR;
  }

  return PARSER_SUCCESS;
}
