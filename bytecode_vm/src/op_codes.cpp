#include "op_codes.hpp"
#include <stdexcept>

Opcode longToOpcode(long val) {
  switch (val) {
  case 0x00:
    return NOP;
  case 0x01:
    return PUSH;
  case 0x02:
    return POP;
  case 0x03:
    return DUP;
  case 0x04:
    return PEEKPRINT;
  case 0x10:
    return ADD;
  case 0x11:
    return SUB;
  case 0x12:
    return MUL;
  case 0x13:
    return DIV;
  case 0x14:
    return CMP;
  case 0x15:
    return AND;
  case 0x16:
    return OR;
  case 0x17:
    return XOR;
  case 0x18:
    return NOT;
  case 0x19:
    return SHL;
  case 0x1A:
    return SHR;
  case 0x20:
    return JMP;
  case 0x21:
    return JZ;
  case 0x22:
    return JNZ;
  case 0x30:
    return STORE;
  case 0x31:
    return LOAD;
  case 0x40:
    return CALL;
  case 0x41:
    return RET;
  case 0xFF:
    return HALT;
  default:
    throw std::runtime_error("Unknown opcode: " + std::to_string(val));
  }
}

std::string opcodeToString(Opcode opcode) {
  switch (opcode) {
  case NOP:
    return "NOP";
  case PUSH:
    return "PUSH";
  case POP:
    return "POP";
  case DUP:
    return "DUP";
  case PEEKPRINT:
    return "PEEKPRINT";
  case ADD:
    return "ADD";
  case SUB:
    return "SUB";
  case MUL:
    return "MUL";
  case DIV:
    return "DIV";
  case CMP:
    return "CMP";
  case AND:
    return "AND";
  case OR:
    return "OR";
  case XOR:
    return "XOR";
  case NOT:
    return "NOT";
  case SHL:
    return "SHL";
  case SHR:
    return "SHR";
  case JMP:
    return "JMP";
  case JZ:
    return "JZ";
  case JNZ:
    return "JNZ";
  case STORE:
    return "STORE";
  case LOAD:
    return "LOAD";
  case CALL:
    return "CALL";
  case RET:
    return "RET";
  case HALT:
    return "HALT";
  default:
    return "UNKNOWN";
  }
}
