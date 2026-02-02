#include "managers/breakpoint_manager.h"

BreakpointManager::BreakpointManager() : bps() {};
BreakpointManager::~BreakpointManager() {};

Breakpoint *BreakpointManager::get(addr_t address) {
  for (size_t i = 0; i < bps.size(); i++) {
    if (bps[i].address == address)
      return &bps[i];
  }
  return nullptr;
}

Breakpoint *BreakpointManager::add(addr_t address) {
  Breakpoint *bpp = get(address);
  if (bpp) {
    bpp->enabled = true;
    return bpp;
  }
  Breakpoint bp = {
      .address = address, .counter = 0, .enabled = true, .original_data = 0};
  bps.push_back(bp);
  return &bps.back();
}

void BreakpointManager::remove(addr_t address) {
  Breakpoint *bpp = get(address);
  if (bpp) {
    bpp->enabled = false;
  }
}

const vector<Breakpoint> &BreakpointManager::get_all_breakpoints() const {
  return bps;
}
