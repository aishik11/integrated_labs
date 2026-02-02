#pragma once
#include "types.h"
#include <vector>
using namespace std;

class BreakpointManager {
public:
  BreakpointManager();
  ~BreakpointManager();
  Breakpoint *get(addr_t address);
  Breakpoint *add(addr_t address);
  void remove(addr_t address);
  const vector<Breakpoint> &get_all_breakpoints() const;

private:
  vector<Breakpoint> bps;
};
