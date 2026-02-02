#include "vm.hpp"
#include "op_codes.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

VM::VM() : pc(0), verbose(false), heap_head(nullptr), num_objects(0) {}

VM::~VM() {
  while (heap_head) {
    Object *next = heap_head->next;
    free(heap_head);
    heap_head = next;
  }
}

void VM::setVerbose(bool v) { verbose = v; }

void VM::load(const std::string &filename) {
  FILE *file = fopen(filename.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("VM Load Error: Could not open file " + filename);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_size == 0) {
    fclose(file);
    throw std::runtime_error("VM Load Error: File is empty.");
  }
  if (file_size % sizeof(long) != 0) {
    fclose(file);
    throw std::runtime_error(
        "VM Load Error: File size is not a multiple of sizeof(long).");
  }

  size_t num_longs = file_size / sizeof(long);
  if (num_longs > MEM_SIZE) {
    fclose(file);
    throw std::runtime_error(
        "VM Load Error: Bytecode size exceeds memory capacity.");
  }

  long buffer[num_longs];
  fread(buffer, sizeof(long), num_longs, file);
  fclose(file);

  program_memory.load(buffer, num_longs);
  pc = 0;
  std::cout << "Loaded " << file_size << " bytes from " << filename
            << std::endl;
}

// --- GC Implementation ---

Object *VM::allocate(ObjectType type) {
  Object *obj = (Object *)malloc(sizeof(Object));
  if (!obj)
    throw std::runtime_error("Heap Allocation Failed");

  obj->marked = false;
  obj->type = type;
  obj->next = heap_head;
  heap_head = obj;
  num_objects++;

  return obj;
}

Object *VM::new_pair(Object *head, Object *tail) {
  Object *obj = allocate(OBJ_PAIR);
  obj->pair.head = head;
  obj->pair.tail = tail;
  return obj;
}

Object *VM::new_function() {
  Object *obj = allocate(OBJ_FUNCTION);
  obj->func.address = 0; // Placeholder
  return obj;
}

Object *VM::new_closure(Object *fn, Object *env) {
  Object *obj = allocate(OBJ_CLOSURE);
  obj->closure.fn = fn;
  obj->closure.env = env;
  return obj;
}

void VM::mark(Object *obj) {
  if (!obj || obj->marked)
    return;

  obj->marked = true;

  switch (obj->type) {
  case OBJ_PAIR:
    mark(obj->pair.head);
    mark(obj->pair.tail);
    break;
  case OBJ_CLOSURE:
    mark(obj->closure.fn);
    mark(obj->closure.env);
    break;
  case OBJ_FUNCTION:
    break;
  }
}

void VM::sweep() {
  Object **curr = &heap_head;
  while (*curr) {
    Object *obj = *curr;
    if (!obj->marked) {
      *curr = obj->next;
      free(obj);
      num_objects--;
    } else {
      obj->marked = false;
      curr = &obj->next;
    }
  }
}

void VM::gc() {
  if (verbose)
    std::cout << "GC Triggered. Objects before: " << num_objects << std::endl;

  for (unsigned long i = 0; i < register_stack.get_size(); ++i) {
    const StackItem &item = register_stack.get_item(i);
    if (item.is_obj) {
      mark((Object *)item.value);
    }
  }

  sweep();

  if (verbose)
    std::cout << "GC Complete. Objects after: " << num_objects << std::endl;
}

void gc(VM &vm) { vm.gc(); }

void VM::run() {
  if (verbose) {
    std::cout << "VM running in verbose mode..." << std::endl;
  } else {
    std::cout << "VM running..." << std::endl;
  }

  while (true) {
    if (pc >= MEM_SIZE) {
      throw std::runtime_error(
          "VM Runtime Error: Program Counter out of bounds.");
    }

    long instruction = program_memory.get(pc++);
    Opcode opcode = longToOpcode(instruction);

    if (verbose) {
      std::cout << "PC: " << pc - 1 << ", Opcode: " << opcodeToString(opcode);
    }

    long val1, val2, addr, idx, amt;
    switch (opcode) {
    case NOP:
      if (verbose)
        std::cout << " (NOP)" << std::endl;
      break;
    case PUSH:
      if (pc >= MEM_SIZE)
        throw std::runtime_error(
            "VM Runtime Error: PUSH operand out of bounds.");
      val1 = program_memory.get(pc++);
      register_stack.push(val1); // Default is_obj=false
      if (verbose)
        std::cout << " " << val1 << " (PUSH " << val1 << ")" << std::endl;
      break;
    case POP:
      register_stack.pop();
      if (verbose)
        std::cout << " (POP)" << std::endl;
      break;
    case DUP:
      register_stack.dup();
      if (verbose)
        std::cout << " (DUP)" << std::endl;
      break;
    case PEEKPRINT:
      val1 = register_stack.peek();
      std::cout << val1 << std::endl;
      if (verbose)
        std::cout << " (PEEKPRINT)" << std::endl;
      break;
    case ADD:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 + val2);
      if (verbose)
        std::cout << " (ADD " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case SUB:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 - val2);
      if (verbose)
        std::cout << " (SUB " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case MUL:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 * val2);
      if (verbose)
        std::cout << " (MUL " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case DIV:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      if (val2 == 0) {
        throw std::runtime_error("VM Runtime Error: Division by zero.");
      }
      register_stack.push(val1 / val2);
      if (verbose)
        std::cout << " (DIV " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case CMP:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 < val2 ? 1 : 0);
      if (verbose)
        std::cout << " (CMP " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case AND:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 & val2);
      if (verbose)
        std::cout << " (AND " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case OR:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 | val2);
      if (verbose)
        std::cout << " (OR " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case XOR:
      val2 = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 ^ val2);
      if (verbose)
        std::cout << " (XOR " << val1 << ", " << val2 << ")" << std::endl;
      break;
    case NOT:
      val1 = register_stack.pop();
      register_stack.push(~val1);
      if (verbose)
        std::cout << " (NOT " << val1 << ")" << std::endl;
      break;
    case SHL:
      amt = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 << amt);
      if (verbose)
        std::cout << " (SHL " << val1 << ", " << amt << ")" << std::endl;
      break;
    case SHR:
      amt = register_stack.pop();
      val1 = register_stack.pop();
      register_stack.push(val1 >> amt);
      if (verbose)
        std::cout << " (SHR " << val1 << ", " << amt << ")" << std::endl;
      break;
    case JMP:
      if (pc >= MEM_SIZE)
        throw std::runtime_error(
            "VM Runtime Error: JMP address out of bounds.");
      addr = program_memory.get(pc++);
      pc = addr;
      if (verbose)
        std::cout << " " << addr << " (JMP to " << addr << ")" << std::endl;
      break;
    case JZ:
      if (pc >= MEM_SIZE)
        throw std::runtime_error("VM Runtime Error: JZ address out of bounds.");
      val1 = register_stack.pop();
      addr = program_memory.get(pc++);
      if (val1 == 0) {
        pc = addr;
      }
      if (verbose)
        std::cout << " " << addr << " (JZ to " << addr << " if " << val1
                  << " == 0)" << std::endl;
      break;
    case JNZ:
      if (pc >= MEM_SIZE)
        throw std::runtime_error(
            "VM Runtime Error: JNZ address out of bounds.");
      val1 = register_stack.pop();
      addr = program_memory.get(pc++);
      if (val1 != 0) {
        pc = addr;
      }
      if (verbose)
        std::cout << " " << addr << " (JNZ to " << addr << " if " << val1
                  << " != 0)" << std::endl;
      break;
    case STORE:
      if (pc >= MEM_SIZE)
        throw std::runtime_error(
            "VM Runtime Error: STORE index out of bounds.");
      val1 = register_stack.pop();
      idx = program_memory.get(pc++);
      data_memory.store(idx, val1);
      if (verbose)
        std::cout << " " << idx << " (STORE " << val1 << " at " << idx << ")"
                  << std::endl;
      break;
    case LOAD:
      if (pc >= MEM_SIZE)
        throw std::runtime_error("VM Runtime Error: LOAD index out of bounds.");
      idx = program_memory.get(pc++);
      register_stack.push(data_memory.get(idx));
      if (verbose)
        std::cout << " " << idx << " (LOAD from " << idx << ")" << std::endl;
      break;
    case CALL:
      if (pc >= MEM_SIZE)
        throw std::runtime_error(
            "VM Runtime Error: CALL address out of bounds.");
      addr = program_memory.get(pc++);
      call_stack.push(pc);
      pc = addr;
      if (verbose)
        std::cout << " " << addr << " (CALL " << addr << ")" << std::endl;
      break;
    case RET:
      pc = call_stack.pop();
      if (verbose)
        std::cout << " (RET to " << pc << ")" << std::endl;
      break;
    case HALT:
      if (verbose)
        std::cout << " (HALT)" << std::endl;
      return;
    default:
      throw std::runtime_error(
          "VM Runtime Error: Unimplemented or unknown opcode: " +
          opcodeToString(opcode));
    }
  }
}

void VM::printStack() {
  std::cout << "Stack (top to bottom):" << std::endl;
  std::vector<long> stack_elements = register_stack.getElements();
  for (int i = stack_elements.size() - 1; i >= 0; --i) {
    std::cout << "  " << stack_elements[i] << std::endl;
  }
  if (stack_elements.empty()) {
    std::cout << "  (empty)" << std::endl;
  }
}
