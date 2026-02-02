#include "stack.hpp"
#include <stdexcept>

Stack::Stack() : ind(0) {}

void Stack::push(long val, bool is_obj) {
  if (is_full())
    throw std::runtime_error("Stack Overflow");
  mem[ind].value = val;
  mem[ind].is_obj = is_obj;
  ind++;
}

long Stack::pop() {
  if (is_empty())
    throw std::runtime_error("Stack Underflow");
  return mem[--ind].value;
}

StackItem Stack::pop_item() {
  if (is_empty())
    throw std::runtime_error("Stack Underflow");
  return mem[--ind];
}

long Stack::peek() {
  if (is_empty())
    throw std::runtime_error("Stack Underflow");
  return mem[ind - 1].value;
}

StackItem Stack::peek_item() {
  if (is_empty())
    throw std::runtime_error("Stack Underflow");
  return mem[ind - 1];
}

bool Stack::is_full() { return ind == STACK_SIZE; }

bool Stack::is_empty() { return ind == 0; }

void Stack::dup() {
  if (is_empty())
    throw std::runtime_error("Stack Underflow: Cannot duplicate from an empty stack.");
  StackItem item = mem[ind - 1];
  push(item.value, item.is_obj);
}

std::vector<long> Stack::getElements() const {
    std::vector<long> elements;
    for (unsigned long i = 0; i < ind; ++i) {
        elements.push_back(mem[i].value);
    }
    return elements;
}