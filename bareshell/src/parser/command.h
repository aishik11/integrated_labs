#ifndef _SRC_PARSER_COMMAND_H_
#define _SRC_PARSER_COMMAND_H_

enum parse_error {
  PARSER_TOKEN_DONE = 0,
  PARSER_SUCCESS,
  PARSER_UNPAIRED_QUOTES,
  PARSER_MALLOC_FAIL,
  PARSER_NO_COMMAND,
  PARSER_MISSING_COMMAND,
  PARSER_NO_IN_FILE,
  PARSER_NO_OUT_FILE,
  PARSER_DUPLICATE_IN_REDIR,
  PARSER_DUPLICATE_OUT_REDIR,
  PARSER_SYNTAX_ERROR,
  PARSER_EOF,
};

extern const char *parse_error_strings[];

#define MAX_TOKEN_COUNT 1000

struct Command {
  char *prog_name;
  char *in_file;
  char *out_file;
  struct Command *next_command;
  char **args;
  int is_background;
};

enum parse_error parse_to_command(char *input_buffer,
                                  struct Command *command_ptr);

#endif // _SRC_PARSER_COMMAND_H_
