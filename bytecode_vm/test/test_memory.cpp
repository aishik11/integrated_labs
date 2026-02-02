#include "../src/memory.hpp"
#include <cassert>
#include <iostream>

void test_memory_init() {
  Memory mem;
  // All memory locations should be initialized to 0
  for (unsigned long i = 0; i < MEM_SIZE; ++i) {
    assert(mem.get(i) == 0 && "Memory not initialized to 0");
  }
  std::cout << "test_memory_init passed" << std::endl;
}

void test_memory_store_get() {
  Memory mem;
  mem.store(0, 100);
  assert(mem.get(0) == 100 && "Memory store/get failed at address 0");

  mem.store(MEM_SIZE - 1, 200);
  assert(mem.get(MEM_SIZE - 1) == 200 && "Memory store/get failed at last address");

  std::cout << "test_memory_store_get passed" << std::endl;
}

void test_memory_load() {
  Memory mem;
  long data[] = {1, 2, 3, 4, 5};
  mem.load(data, 5);

  for (int i = 0; i < 5; ++i) {
    assert(mem.get(i) == data[i] && "Memory load failed");
  }
  std::cout << "test_memory_load passed" << std::endl;
}

void test_memory_reset() {
  Memory mem;
  mem.store(0, 100);
  mem.store(1, 200);
  mem.reset();
  assert(mem.get(0) == 0 && "Memory reset failed at address 0");
  assert(mem.get(1) == 0 && "Memory reset failed at address 1");
  std::cout << "test_memory_reset passed" << std::endl;
}


int main() {
  test_memory_init();
  test_memory_store_get();
  test_memory_load();
  test_memory_reset();
  return 0;
}
