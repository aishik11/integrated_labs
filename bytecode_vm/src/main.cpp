#include "vm.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  std::string filename;
  bool verbose = false;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <bytecode_file> [--verbose]"
              << std::endl;
    return 1;
  }

  filename = argv[1];

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose" || arg == "-v") {
      verbose = true;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return 1;
    }
  }

  VM vm;
  vm.setVerbose(verbose);

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
