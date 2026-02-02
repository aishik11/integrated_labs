#pragma once

typedef int pid_t;
typedef unsigned long addr_t;

typedef struct {
  addr_t address;
  int counter;
  bool enabled;
  long original_data;
} Breakpoint;
