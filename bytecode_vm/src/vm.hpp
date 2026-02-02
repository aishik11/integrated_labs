#ifndef VM_H
#define VM_H

#include "memory.hpp"
#include "object.hpp"
#include "stack.hpp"
#include <string>

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
  void setVerbose(bool v);
  void printStack();

private:
};

void gc(VM &vm);

#endif // !VM_H
