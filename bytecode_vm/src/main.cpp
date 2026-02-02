#include "vm.hpp"
#include <iostream>
#include <string>

#include <csignal>

VM *global_vm = nullptr;

void handle_signal(int signal) {
  if (signal == SIGUSR1 && global_vm) {
    global_vm->debug_mode = true;
    // We don't need to do anything else; the run loop checks this flag
  }
}

int main(int argc, char *argv[]) {
  std::string filename;
  bool verbose = false;
  bool debug = false;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <bytecode_file> [--verbose] [--debug]"
              << std::endl;
    return 1;
  }

  filename = argv[1];

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose" || arg == "-v") {
      verbose = true;
    } else if (arg == "--debug" || arg == "-d") {
      debug = true;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return 1;
    }
  }

  VM vm;
  vm.setVerbose(verbose);
  vm.debug_mode = debug;
  
  global_vm = &vm;
  signal(SIGUSR1, handle_signal);
  
  // Unblock SIGUSR1 now that handler is installed
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  try {
    vm.load(filename);
    vm.run();
    if (verbose) {
      vm.printStack();
    }
  } catch (const std::runtime_error &e) {
    std::cerr << "VM Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
