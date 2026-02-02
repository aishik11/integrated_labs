#include "runner/debugger.h"
#include "types.h"
#include "utils/parsers.h"
#include <cstdio>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

using std::cout, std::cerr, std::cin, std::endl;
using std::string;
using std::vector;

void Debugger::load(const std::string &program_path) {
  if (!this->target_path.empty()) {
    cerr << "Error: A binary is already loaded. Please unload it first."
         << endl;
    return;
  }

  if (access(program_path.c_str(), F_OK) == -1) {
    cerr << "Error: Binary not found: " << program_path << endl;
    return;
  }

  if (access(program_path.c_str(), X_OK) == -1) {
    cerr << "Error: Binary is not executable: " << program_path << endl;
    return;
  }

  this->target_path = program_path;
  _load_binary();
}

int Debugger::_load_binary() {
  ptracer.load_binary(this->target_path.c_str());
  return 0; // Assuming success for now
}

void Debugger::attach(pid_t pid) {
  ptracer.attach_to_process(pid);
  if (ptracer.is_loaded_status()) {
    this->target_path = "[attached_process]"; // Placeholder
  }
}

void Debugger::run() {

  while (true) {
    repl();
  }
}

string Debugger::read_command() {
  string command;
  getline(cin, command);
  if (cin.eof()) {
    return "quit";
  }
  return command;
}
void Debugger::display_prompt() { cout << "dbg> "; }

void Debugger::print_help() {
  cout << "Available commands:" << endl;
  cout << "  q[uit] | exit                      : Exit the debugger" << endl;
  cout << "  h[elp]                             : Display this help message"
       << endl;
  cout << "  l[oad] <path>                      : Load a binary into the "
          "debugger"
       << endl;
  cout << "  b[reak] <address>                  : Set a breakpoint" << endl;
  cout << "  rb[p] | remove_b[reak] <address>   : Remove a breakpoint" << endl;
  cout << "  list_b[p] | info_b[p] | ib         : List breakpoints" << endl;
  cout << "  s[tep] | nexti                     : Execute one instruction"
       << endl;
  cout << "  c[ontinue]                         : Continue execution" << endl;
  cout << "  r[un]                              : Run the loaded binary"
       << endl;
  cout << "  reg[s] | ir | info_r[egs]          : Print register information"
       << endl;
  cout << "  status | ps                        : Report debugger status"
       << endl;
}

void Debugger ::repl() {
  display_prompt();
  string command = read_command();
  parse_and_execute_cmd(command);
}

void Debugger::parse_and_execute_cmd(const string &command) {
  auto tokens = get_tokens(command);
  if (tokens.size() == 0)
    return;
  string cmd = tokens[0];
  if (cmd.rfind("q", 0) == 0 || cmd == "exit") {
    ptracer.kill_child_process();
    exit(0);
  } else if (cmd.rfind("h", 0) == 0) {
    print_help();
  } else if (cmd.rfind("l", 0) == 0) {
    if (tokens.size() > 1) {
      this->load(tokens[1]);
    } else {
      cerr << "Error: `load` command requires a binary path." << endl;
    }

  } else if (cmd.rfind("b", 0) == 0) {
    if (this->target_path.empty()) {
      cerr << "Error: No binary loaded. Please load a binary first." << endl;
      return;
    }
    if (tokens.size() > 1) {
      try {
        addr_t addr = string_to_address(tokens[1]);
        ptracer.add_breakpoint(addr);
        cout << "Breakpoint set at 0x" << std::hex << addr << std::dec << endl;
      } catch (const std::invalid_argument &e) {
        cerr << "Error: Invalid address format for breakpoint. " << e.what()
             << endl;
      }
    } else {
      cerr << "Error: `break` command requires an address." << endl;
    }

  } else if (cmd.rfind("rb", 0) == 0 || cmd.rfind("remove_b", 0) == 0) {
    if (this->target_path.empty()) {
      cerr << "Error: No binary loaded. Please load a binary first." << endl;
      return;
    }
    if (tokens.size() > 1) {
      try {
        addr_t addr = string_to_address(tokens[1]);
        if (ptracer.remove_breakpoint(addr) == 0) {
          cout << "Breakpoint removed from 0x" << std::hex << addr << std::dec
               << endl;
        } else {
          cerr << "Error: Breakpoint not found at 0x" << std::hex << addr
               << std::dec << endl;
        }
      } catch (const std::invalid_argument &e) {
        cerr << "Error: Invalid address format for breakpoint removal. "
             << e.what() << endl;
      }
    } else {
      cerr << "Error: `remove_break` command requires an address." << endl;
    }
  } else if (cmd.rfind("list_b", 0) == 0 || cmd.rfind("info_b", 0) == 0 ||
             cmd == "ib") {
    if (this->target_path.empty()) {
      cerr << "Error: No binary loaded. Please load a binary first." << endl;
      return;
    }
    ptracer.list_breakpoints();
  } else if (cmd.rfind("s", 0) == 0 || cmd == "nexti") {
    if (!ptracer.is_child_alive_status()) {
      cerr << "Error: No active process to step. Please load and run a binary "
              "first."
           << endl;
      return;
    }

    printf("Stepping...\n");
    Breakpoint current_breakpoint = ptracer.get_current_breakpoint();
    if (ptracer.execute_single_step(current_breakpoint) == 0) {

      printf("\nSingle step ran successfully.\n");
    }

  } else if (cmd.rfind("c", 0) == 0) {
    if (!ptracer.is_child_alive_status()) {
      cerr << "Error: No active process to continue. Please load and run a "
              "binary first."
           << endl;
      return;
    }

    printf("Continuing...\n");
    Breakpoint current_breakpoint = ptracer.get_current_breakpoint();
    if (ptracer.continue_execution(current_breakpoint) == 0) {

      printf("\nContinued successfully.\n");
    }

  } else if (cmd.rfind("reg", 0) == 0 || cmd == "ir" ||
             cmd.rfind("info_r", 0) == 0) {
    if (this->target_path.empty()) {
      cerr << "Error: No binary loaded. Please load a binary first." << endl;
      return;
    }

    ptracer.print_registers();
  } else if (cmd.rfind("r", 0) == 0) {
    if (this->target_path.empty()) {
      cerr << "Error: No binary loaded. Please load a binary first." << endl;
      return;
    }
    printf("Running...\n");
    ptracer.continue_process();
  } else if (cmd == "status" || cmd == "ps") {
    cout << "--- Debugger Status ---" << endl;
    if (this->target_path.empty()) {
      cout << "No binary loaded." << endl;
    } else {
      cout << "Loaded Binary: " << this->target_path << endl;
      if (ptracer.is_child_alive_status()) {
        cout << "Child PID: " << ptracer.get_child_id() << endl;
        cout << "Process Status: Running"
             << endl; // Or "Stopped at breakpoint" if applicable
      } else {
        cout << "Child PID: N/A (process not active or exited)" << endl;
        cout << "Process Status: Not Running / Exited" << endl;
      }
    }
    cout << "-----------------------" << endl;
  } else {
    cerr << "Error: command '" << cmd << "' not recognized." << endl;
    print_help();
  }
}
