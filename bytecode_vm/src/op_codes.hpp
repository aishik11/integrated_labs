#ifndef OPCODE_H
#define OPCODE_H

#include <string>

typedef enum Opcode {
  // Blank
  NOP = 0x00,
  // Data
  PUSH,
  POP,
  DUP,
  PEEKPRINT = 0x04,
  // Arithmetic
  ADD = 0x10,
  SUB,
  MUL,
  DIV,
  CMP,
  // Bitwise
  AND = 0x15,
  OR,
  XOR,
  NOT,
  SHL,
  SHR,
  // Control
  JMP = 0x20,
  JZ,
  JNZ,
  // Memory
  STORE = 0x30,
  LOAD,
  // Control flow - functions
  CALL = 0x40,
  RET,
  // Halt
  HALT = 0xFF,
} Opcode;

Opcode longToOpcode(long val);
std::string opcodeToString(Opcode opcode);

#endif // !OPCODE_H
