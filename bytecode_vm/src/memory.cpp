#include "memory.hpp"
#include <cstring>
#include <stdexcept>

Memory::Memory() { memset(mem, 0, sizeof(mem)); }

Memory::~Memory() {}

bool Memory::is_valid_address(unsigned long address) {
  return address < MEM_SIZE;
}

void Memory::load(long array[], long size) {
  if (size > MEM_SIZE)
    throw std::runtime_error(
        "Memory Load Error: Array size exceeds memory capacity.");
  for (long i = 0; i < size; ++i) {
    mem[i] = array[i];
  }
}

void Memory::reset() { memset(mem, 0, sizeof(mem)); }

void Memory::store(unsigned long address, long val) {
  if (!is_valid_address(address))
    throw std::runtime_error("Memory Store Error: Invalid memory address.");
  mem[address] = val;
}

long Memory::get(unsigned long address) {
  if (!is_valid_address(address))
    throw std::runtime_error("Memory Get Error: Invalid memory address.");
  return mem[address];
}
