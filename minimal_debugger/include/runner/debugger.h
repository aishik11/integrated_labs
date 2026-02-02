#pragma once

#include "types.h"
#include <runner/ptrace_wrapper.h>
#include <string>
#include <vector>
using std::string;

class Debugger {
public:
  Debugger() = default;
  ~Debugger() = default;
  void run();
  void load(const std::string &program_path);
  void attach(pid_t pid);

  std::string read_command();
  void display_prompt();
  void repl();
  void parse_and_execute_cmd(const std::string &command);
  void print_help();

private:
  ptrace_wrapper ptracer;
  std::string target_path;
  int _load_binary();
};
