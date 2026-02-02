#include "../src/op_codes.hpp"
#include <cassert>
#include <iostream>

void test_longToOpcode() {
  // Test valid opcodes
  assert(longToOpcode(0x00) == NOP && "longToOpcode NOP failed");
  assert(longToOpcode(0x01) == PUSH && "longToOpcode PUSH failed");
  assert(longToOpcode(0x10) == ADD && "longToOpcode ADD failed");
  assert(longToOpcode(0x20) == JMP && "longToOpcode JMP failed");
  assert(longToOpcode(0x30) == STORE && "longToOpcode STORE failed");
  assert(longToOpcode(0xFF) == HALT && "longToOpcode HALT failed");

  // Test an unknown opcode (should throw an exception)
  bool caught_exception = false;
  try {
    longToOpcode(0x05); // An undefined opcode
  } catch (const std::runtime_error &e) {
    caught_exception = true;
  }
  assert(caught_exception && "longToOpcode did not throw for unknown opcode");

  std::cout << "test_longToOpcode passed" << std::endl;
}

void test_opcodeToString() {
  // Test known opcodes
  assert(opcodeToString(NOP) == "NOP" && "opcodeToString NOP failed");
  assert(opcodeToString(PUSH) == "PUSH" && "opcodeToString PUSH failed");
  assert(opcodeToString(ADD) == "ADD" && "opcodeToString ADD failed");
  assert(opcodeToString(JMP) == "JMP" && "opcodeToString JMP failed");
  assert(opcodeToString(STORE) == "STORE" && "opcodeToString STORE failed");
  assert(opcodeToString(HALT) == "HALT" && "opcodeToString HALT failed");

  std::cout << "test_opcodeToString passed" << std::endl;
}

int main() {
  test_longToOpcode();
  test_opcodeToString();
  return 0;
}
