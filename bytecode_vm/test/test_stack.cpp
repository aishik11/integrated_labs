#include "../src/stack.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_push_pop() {
  Stack s;
  s.push(10);
  assert(s.pop() == 10);
  std::cout << "test_push_pop passed" << std::endl;
}

void test_is_empty_full() {
  Stack s;
  assert(s.is_empty() == true);
  s.push(1);
  assert(s.is_empty() == false);
  s.pop();
  assert(s.is_empty() == true);

  for (int i = 0; i < STACK_SIZE; ++i) {
    s.push(i);
  }
  assert(s.is_full() == true);
  std::cout << "test_is_empty_full passed" << std::endl;
}

void test_stack_overflow() {
  Stack s;
  bool caught_exception = false;
  try {
    for (int i = 0; i < STACK_SIZE; ++i) {
      s.push(i);
    }
    s.push(100); // This should cause an overflow
  } catch (const std::runtime_error &e) {
    if (std::string(e.what()) == "Stack Overflow") {
      caught_exception = true;
    }
  }
  assert(caught_exception == true);
  std::cout << "test_stack_overflow passed" << std::endl;
}

void test_stack_underflow() {
  Stack s;
  bool caught_exception = false;
  try {
    s.pop(); // This should cause an underflow
  } catch (const std::runtime_error &e) {
    if (std::string(e.what()) == "Stack Underflow") {
      caught_exception = true;
    }
  }
  assert(caught_exception == true);
  std::cout << "test_stack_underflow passed" << std::endl;
}

int main() {
  test_push_pop();
  test_is_empty_full();
  test_stack_overflow();
  test_stack_underflow();
  std::cout << "All Stack tests passed!" << std::endl;
  return 0;
}
