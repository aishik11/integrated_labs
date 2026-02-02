#ifndef STACK_H
#define STACK_H

#include <vector>

#define STACK_SIZE 1024 * 10

struct StackItem {
  long value;
  bool is_obj;
};

class Stack {
public:
  Stack();
  ~Stack() = default;
  void push(long val, bool is_obj = false);
  long pop();
  StackItem pop_item();
  long peek();
  StackItem peek_item();
  void dup();
  bool is_empty();
  bool is_full();
  std::vector<long> getElements() const;

  // For GC access
  unsigned long get_size() const { return ind; }
  const StackItem &get_item(unsigned long i) const { return mem[i]; }

private:
  StackItem mem[STACK_SIZE];
  unsigned long ind = 0;
};

#endif // !STACK_H
