#include "../src/vm.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Helper function to create a bytecode file with long values
void create_bytecode_file(const std::string &filename,
                          const std::vector<long> &bytecode) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to create bytecode file: " + filename);
  }
  file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size() * sizeof(long));
  file.close();
}

void test_vm_push_add_halt() {
  std::cout << "Running test_vm_push_add_halt..." << std::endl;
  std::string test_file = "test_push_add_halt.bin";
  // PUSH 5, PUSH 3, ADD, HALT (long-based opcodes and operands)
  create_bytecode_file(test_file, {0x01, 5, 0x01, 3, 0x10, 0xFF}); 

  VM vm;
  vm.setVerbose(false); // Disable verbose for tests unless specifically needed
  vm.load(test_file);
  vm.run();

  assert(vm.register_stack.pop() == 8 && "PUSH 5, PUSH 3, ADD did not result in 8");
  assert(vm.register_stack.is_empty() && "Register stack not empty after test");
  std::cout << "test_vm_push_add_halt passed" << std::endl;

  remove(test_file.c_str()); // Clean up
}

void test_vm_store_load_halt() {
  std::cout << "Running test_vm_store_load_halt..." << std::endl;
  std::string test_file = "test_store_load_halt.bin";
  // PUSH 10, STORE 0, LOAD 0, HALT (long-based opcodes and operands)
  create_bytecode_file(test_file, {0x01, 10, 0x30, 0, 0x31, 0, 0xFF}); 

  VM vm;
  vm.setVerbose(false);
  vm.load(test_file);
  vm.run();

  assert(vm.register_stack.pop() == 10 && "STORE 10 at 0, LOAD from 0 did not result in 10");
  assert(vm.register_stack.is_empty() && "Register stack not empty after test");
  std::cout << "test_vm_store_load_halt passed" << std::endl;

  remove(test_file.c_str()); // Clean up
}


int main() {
  try {
    test_vm_push_add_halt();
    test_vm_store_load_halt();
  } catch (const std::runtime_error &e) {
    std::cerr << "Test Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
