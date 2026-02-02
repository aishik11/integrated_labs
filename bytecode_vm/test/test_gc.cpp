#include "../src/vm.hpp"
#include <cassert>
#include <iostream>

#define VAL_OBJ(o) (o)

void push(VM &vm, Object *o) { vm.register_stack.push((long)o, true); }

void test_basic_reachability() {
  std::cout << "Running test_basic_reachability..." << std::endl;
  VM vm;
  Object *a = vm.new_pair(nullptr, nullptr);
  push(vm, VAL_OBJ(a));

  vm.gc();

  bool found = false;
  Object *curr = vm.heap_head;
  while (curr) {
    if (curr == a)
      found = true;
    curr = curr->next;
  }

  assert(found && "Object a should survive");
  assert(vm.num_objects == 1 && "Heap count mismatch");
  std::cout << "test_basic_reachability passed." << std::endl;
}

void test_unreachable() {
  std::cout << "Running test_unreachable..." << std::endl;
  VM vm;
  Object *a = vm.new_pair(nullptr, nullptr);
  (void)a;

  vm.gc();

  assert(vm.heap_head == nullptr && "Heap should be empty");
  assert(vm.num_objects == 0 && "Heap count mismatch");
  std::cout << "test_unreachable passed." << std::endl;
}

void test_transitive() {
  std::cout << "Running test_transitive..." << std::endl;
  VM vm;
  Object *a = vm.new_pair(nullptr, nullptr);
  Object *b = vm.new_pair(a, nullptr);

  push(vm, VAL_OBJ(b));

  vm.gc();

  assert(vm.num_objects == 2 && "Both objects should survive");
  std::cout << "test_transitive passed." << std::endl;
}

void test_cyclic() {
  std::cout << "Running test_cyclic..." << std::endl;
  VM vm;
  Object *a = vm.new_pair(nullptr, nullptr);
  Object *b = vm.new_pair(a, nullptr);

  a->pair.tail = b;

  push(vm, VAL_OBJ(a));

  vm.gc();

  assert(vm.num_objects == 2 && "Both objects in cycle should survive");
  std::cout << "test_cyclic passed." << std::endl;
}

void test_deep_graph() {
  std::cout << "Running test_deep_graph..." << std::endl;
  VM vm;
  Object *root = vm.new_pair(nullptr, nullptr);
  Object *cur = root;
  int depth = 10000;

  for (int i = 0; i < depth; ++i) {
    Object *next = vm.new_pair(nullptr, nullptr);
    cur->pair.tail = next;
    cur = next;
  }

  push(vm, VAL_OBJ(root));

  vm.gc();

  assert(vm.num_objects == (size_t)(depth + 1) && "All objects should survive");
  std::cout << "test_deep_graph passed." << std::endl;
}

void test_closure_capture() {
  std::cout << "Running test_closure_capture..." << std::endl;
  VM vm;
  Object *env = vm.new_pair(nullptr, nullptr);
  Object *fn = vm.new_function();
  Object *cl = vm.new_closure(fn, env);

  push(vm, VAL_OBJ(cl));

  vm.gc();

  assert(vm.num_objects == 3 &&
         "Closure, Function, and Environment should survive");
  std::cout << "test_closure_capture passed." << std::endl;
}

void test_stress_allocation() {
  std::cout << "Running test_stress_allocation..." << std::endl;
  VM vm;

  for (int i = 0; i < 100000; ++i) {
    vm.new_pair(nullptr, nullptr);
  }

  vm.gc();

  assert(vm.num_objects == 0 && "Heap should be empty after stress GC");
  std::cout << "test_stress_allocation passed." << std::endl;
}

int main() {
  try {
    test_basic_reachability();
    test_unreachable();
    test_transitive();
    test_cyclic();
    test_deep_graph();
    test_closure_capture();
    test_stress_allocation();
    std::cout << "All GC tests passed!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Test Failed: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
