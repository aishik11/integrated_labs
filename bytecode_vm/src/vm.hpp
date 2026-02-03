#ifndef VM_H
#define VM_H

#include "memory.hpp"
#include "object.hpp"
#include "stack.hpp"
#include <string>
#include <set>

class VM {
public:
  VM();
  ~VM();

  Stack call_stack;
  Stack register_stack;
  Memory program_memory;
  Memory data_memory;
  unsigned long pc;
  bool verbose;
  bool debug_mode;
  std::set<unsigned long> breakpoints;

  Object *heap_head;
  size_t num_objects;

  Object *allocate(ObjectType type);
  Object *new_pair(Object *head, Object *tail);
  Object *new_function();
  Object *new_closure(Object *fn, Object *env);

  void mark(Object *obj);
  void sweep();
  void gc();

  void load(const std::string &filename);
  void run();
  void run_debug(); // Main loop variant for debug mode
  void repl();      // Read-Eval-Print Loop for debug commands
  void step();      // Execute single instruction
  void setVerbose(bool v);
  void printStack();
  void printStats();
  
  bool stats_requested;

private:
};

void gc(VM &vm);

#endif // !VM_H
