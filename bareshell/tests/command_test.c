#include "../src/parser/command.h"
#include "minunit.h"
#include <stdlib.h>
#include <string.h>

#define mu_assert_parse_error_eq(expected_err, actual_err)                     \
  MU__SAFE_BLOCK(enum parse_error _expected = (expected_err);                  \
                 enum parse_error _actual = (actual_err);                      \
                 mu_assert_string_eq(parse_error_strings[_expected],           \
                                     parse_error_strings[_actual]);)

MU_TEST(test_parse_simple_command) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("ls", cmd.prog_name);
  mu_assert_string_eq("ls", cmd.args[0]);
  mu_assert(cmd.args[1] == NULL, "args should be empty");
  mu_assert(cmd.in_file == NULL, "no input file");
  mu_assert(cmd.out_file == NULL, "Expected no output redirection");
  mu_assert(cmd.next_command == NULL, "Expected no pipe/next command");
}

MU_TEST(test_parse_command_with_args) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls -l -a", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("ls", cmd.prog_name);
  mu_assert_string_eq("ls", cmd.args[0]);
  mu_assert_string_eq("-l", cmd.args[1]);
  mu_assert_string_eq("-a", cmd.args[2]);
  mu_assert(cmd.args[3] == NULL,
            "Expected arguments list to terminate at index 3");
}

MU_TEST(test_parse_command_with_output_redir) {
  struct Command cmd;
  enum parse_error err = parse_to_command("echo hello > out.txt", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("echo", cmd.prog_name);
  mu_assert_string_eq("echo", cmd.args[0]);
  mu_assert_string_eq("hello", cmd.args[1]);
  mu_assert(cmd.args[2] == NULL,
            "Expected arguments list to terminate at index 2");
  mu_assert_string_eq("out.txt", cmd.out_file);
}

MU_TEST(test_parse_command_with_input_redir) {
  struct Command cmd;
  enum parse_error err = parse_to_command("cat < in.txt", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("cat", cmd.prog_name);
  mu_assert_string_eq("cat", cmd.args[0]);
  mu_assert(cmd.args[1] == NULL,
            "Expected arguments list to terminate at index 1");
  mu_assert_string_eq("in.txt", cmd.in_file);
}

MU_TEST(test_parse_command_with_pipe) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls | grep foo", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("ls", cmd.prog_name);
  mu_assert_string_eq("ls", cmd.args[0]);
  mu_assert(cmd.args[1] == NULL,
            "Expected arguments list to terminate at index 1");
  mu_assert(cmd.next_command != NULL, "Expected a pipe/next command");
  mu_assert_string_eq("grep", cmd.next_command->prog_name);
  mu_assert_string_eq("grep", cmd.next_command->args[0]);
  mu_assert_string_eq("foo", cmd.next_command->args[1]);
  mu_assert(cmd.next_command->args[2] == NULL,
            "Expected arguments list to terminate at index 2 for next command");
}

MU_TEST(test_parse_quoted_string) {
  struct Command cmd;
  enum parse_error err = parse_to_command("echo \"hello world\"", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("echo", cmd.prog_name);
  mu_assert_string_eq("echo", cmd.args[0]);
  mu_assert_string_eq("\"hello world\"", cmd.args[1]);
  mu_assert(cmd.args[2] == NULL,
            "Expected arguments list to terminate at index 2");
}

MU_TEST(test_parse_unpaired_quotes) {
  struct Command cmd;
  enum parse_error err = parse_to_command("echo \"hello", &cmd);
  mu_assert_parse_error_eq(PARSER_UNPAIRED_QUOTES, err);
}

MU_TEST(test_parse_duplicate_output_redir) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls > a > b", &cmd);
  mu_assert_parse_error_eq(PARSER_DUPLICATE_OUT_REDIR, err);
}

MU_TEST(test_parse_no_output_file) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls >", &cmd);
  mu_assert_parse_error_eq(PARSER_NO_OUT_FILE, err);
}

MU_TEST(test_parse_no_input_file) {
  struct Command cmd;
  enum parse_error err = parse_to_command("cat <", &cmd);
  mu_assert_parse_error_eq(PARSER_NO_IN_FILE, err);
}

MU_TEST(test_parse_missing_command) {
  struct Command cmd;
  enum parse_error err = parse_to_command("> out.txt", &cmd);
  mu_assert_parse_error_eq(PARSER_MISSING_COMMAND, err);
}

MU_TEST(test_parse_duplicate_input_redir) {
  struct Command cmd;
  enum parse_error err = parse_to_command("cat < a < b", &cmd);
  mu_assert_parse_error_eq(PARSER_DUPLICATE_IN_REDIR, err);
}

MU_TEST(test_parse_no_command) {
  struct Command cmd;
  enum parse_error err = parse_to_command("", &cmd);
  mu_assert_parse_error_eq(PARSER_NO_COMMAND, err);
}

MU_TEST(test_parse_background_command) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls &", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("ls", cmd.prog_name);
  mu_assert_string_eq("ls", cmd.args[0]);
  mu_assert(cmd.args[1] == NULL, "args should be empty");
  mu_assert(cmd.is_background == 1, "Command should be in background");
  mu_assert(cmd.next_command == NULL, "Expected no pipe/next command");
}

MU_TEST(test_parse_background_pipeline) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls | wc &", &cmd);
  mu_assert_parse_error_eq(PARSER_SUCCESS, err);
  mu_assert_string_eq("ls", cmd.prog_name);
  mu_assert(cmd.is_background == 1, "First command should be in background");
  mu_assert(cmd.next_command != NULL, "Expected a pipe/next command");
  mu_assert_string_eq("wc", cmd.next_command->prog_name);
  mu_assert(cmd.next_command->is_background == 1,
            "Second command should be in background");
}

MU_TEST(test_parse_background_syntax_error) {
  struct Command cmd;
  enum parse_error err = parse_to_command("ls & wc", &cmd);
  mu_assert_parse_error_eq(PARSER_SYNTAX_ERROR, err);
}

MU_TEST(test_parse_background_missing_command) {
  struct Command cmd;
  enum parse_error err = parse_to_command("& ls", &cmd);
  mu_assert_parse_error_eq(PARSER_MISSING_COMMAND, err);
}

MU_TEST_SUITE(command_suite) {
  MU_RUN_TEST(test_parse_simple_command);
  MU_RUN_TEST(test_parse_command_with_args);
  MU_RUN_TEST(test_parse_command_with_output_redir);
  MU_RUN_TEST(test_parse_command_with_input_redir);
  MU_RUN_TEST(test_parse_command_with_pipe);
  MU_RUN_TEST(test_parse_quoted_string);
  MU_RUN_TEST(test_parse_unpaired_quotes);
  MU_RUN_TEST(test_parse_duplicate_output_redir);
  MU_RUN_TEST(test_parse_no_output_file);
  MU_RUN_TEST(test_parse_no_input_file);
  MU_RUN_TEST(test_parse_missing_command);
  MU_RUN_TEST(test_parse_duplicate_input_redir);
  MU_RUN_TEST(test_parse_no_command);
  MU_RUN_TEST(test_parse_background_command);
  MU_RUN_TEST(test_parse_background_pipeline);
  MU_RUN_TEST(test_parse_background_syntax_error);
  MU_RUN_TEST(test_parse_background_missing_command);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  MU_RUN_SUITE(command_suite);
  MU_REPORT();
  return MU_EXIT_CODE;
}
