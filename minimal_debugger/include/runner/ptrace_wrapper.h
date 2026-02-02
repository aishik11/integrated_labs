#pragma once
#include "managers/breakpoint_manager.h"
#include <string>
#include <sys/user.h>
using std::string;
#include "types.h"

class ptrace_wrapper {
private:
  pid_t child_id = -1;
  bool is_loaded = false;
  bool is_child_alive = false;
  int status;
  struct user_regs_struct regs;
  BreakpointManager breakpoint_manager;

public:
  ptrace_wrapper() = default;
  ~ptrace_wrapper() = default;
  int load_binary(const char *program_name);
  int add_breakpoint(addr_t addr);
  int remove_breakpoint(addr_t address);
  int execute_single_step(Breakpoint breakpoint);
  int continue_execution(Breakpoint breakpoint);
  Breakpoint get_current_breakpoint();
  void print_registers();
  void list_breakpoints();
  void continue_process();
  void kill_child_process();
  void attach_to_process(pid_t pid);
  bool is_loaded_status() const; // New getter
  int get_child_id() const { return child_id; }
  bool is_child_alive_status() const { return is_child_alive; }

};
