#include "runner/ptrace_wrapper.h"
#include "types.h"
#include "utils/parsers.h"
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

int ptrace_wrapper::load_binary(const char *program_name) {

  int child = fork();

  if (child == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl(program_name, program_name, NULL);
  } else {
    is_loaded = true;
    child_id = child;
    is_child_alive = true;

    waitpid(child, &status, 0);

    return child;
  }

  return -1;
};

void ptrace_wrapper::print_registers() {
  ptrace(PTRACE_GETREGS, child_id, NULL, &regs);

  printf("\n--- CPU Registers ---\n");
  printf("RIP: 0x%llx\n", regs.rip);
  printf("RAX: 0x%llx  RBX: 0x%llx  RCX: 0x%llx  RDX: 0x%llx\n", regs.rax,
         regs.rbx, regs.rcx, regs.rdx);
  printf("RSI: 0x%llx  RDI: 0x%llx  RBP: 0x%llx  RSP: 0x%llx\n", regs.rsi,
         regs.rdi, regs.rbp, regs.rsp);
  printf("R8 : 0x%llx  R9 : 0x%llx  R10: 0x%llx  R11: 0x%llx\n", regs.r8,
         regs.r9, regs.r10, regs.r11);
  printf("---------------------\n");
}

int ptrace_wrapper::add_breakpoint(addr_t addr) {
  if (!is_loaded) {
    return -1;
  } else {
    long orig_data = ptrace(PTRACE_PEEKTEXT, child_id, (void *)addr, NULL);
    Breakpoint *bp = breakpoint_manager.add(addr);
    bp->original_data = orig_data;
    long trap_data = (orig_data & ~0xFF) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_id, (void *)addr, (void *)trap_data);

    printf("Breakpoint set.\n");
    // ptrace(PTRACE_CONT, child_id, NULL, NULL);

    return 0;
  }

  return -1;
};

int ptrace_wrapper::remove_breakpoint(addr_t address) {
  Breakpoint *bp = breakpoint_manager.get(address);
  if (bp) {
    ptrace(PTRACE_POKETEXT, child_id, (void *)bp->address,
           (void *)bp->original_data);
    breakpoint_manager.remove(address);
    return 0;
  }
  return -1;
}

Breakpoint ptrace_wrapper::get_current_breakpoint() {
  ptrace(PTRACE_GETREGS, child_id, NULL, &regs);
  addr_t actual_breakpoint_address = regs.rip - 1;

  Breakpoint *bp = breakpoint_manager.get(actual_breakpoint_address);
  if (bp) {
    return *bp;
  } else {
    long next_data = ptrace(PTRACE_PEEKTEXT, child_id,
                            (void *)actual_breakpoint_address, NULL);
    Breakpoint dummy_bp;
    dummy_bp.address = actual_breakpoint_address;
    dummy_bp.original_data = next_data;
    dummy_bp.enabled = false;
    dummy_bp.counter = 0;
    return dummy_bp;
  }
}

int ptrace_wrapper::execute_single_step(Breakpoint breakpoint) {
  if (!is_child_alive) {
    printf("Error: Child process is not alive.\n");
    return -1;
  }

  if (breakpoint.enabled) {
    ptrace(PTRACE_POKETEXT, child_id, (void *)breakpoint.address,
           (void *)breakpoint.original_data);

    ptrace(PTRACE_GETREGS, child_id, NULL, &regs);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_id, NULL, &regs);

    ptrace(PTRACE_SINGLESTEP, child_id, NULL, NULL);
    waitpid(child_id, &status, 0);
    if (WIFEXITED(status)) {
      is_child_alive = false;
      printf("Child process exited with status %d.\n", WEXITSTATUS(status));
      return 0;
    }

    long trap_data = (breakpoint.original_data & ~0xFF) | 0xCC;

    ptrace(PTRACE_POKETEXT, child_id, (void *)breakpoint.address,
           (void *)trap_data);
  } else {
    // Not at a breakpoint, just step
    ptrace(PTRACE_SINGLESTEP, child_id, NULL, NULL);
    waitpid(child_id, &status, 0);
    if (WIFEXITED(status)) {
      is_child_alive = false;
      printf("Child process exited with status %d.\n", WEXITSTATUS(status));
      return 0;
    }
  }

  return 0;
}

int ptrace_wrapper::continue_execution(Breakpoint breakpoint) {
  if (!is_child_alive) {
    printf("Error: Child process is not alive.\n");
    return -1;
  }
  execute_single_step(breakpoint);
  printf("[Running] Resuming execution...\n");
  ptrace(PTRACE_CONT, child_id, NULL, NULL);

  waitpid(child_id, &status, 0);
  if (WIFEXITED(status)) {
    is_child_alive = false;
    printf("\nChild process exited with status %d.\n", WEXITSTATUS(status));
  } else if (WIFSTOPPED(status)) {
    int sig = WSTOPSIG(status);
    if (sig == SIGINT) {
      printf("\nProcess stopped by SIGINT.\n");
    } else if (sig == SIGTRAP) {
      printf("\nProcess stopped by SIGTRAP (Breakpoint).\n");
    } else {
      printf("\nProcess stopped by signal %d.\n", sig);
    }
  }
  return 0;
}

void ptrace_wrapper::list_breakpoints() {
  const std::vector<Breakpoint> &bps = breakpoint_manager.get_all_breakpoints();
  if (bps.empty()) {
    printf("No breakpoints set.\n");
    return;
  }

  printf("Breakpoints:\n");
  printf("Address\t\tEnabled\t\tCounter\n");
  printf("-------\t\t-------\t\t-------\n");
  for (const auto &bp : bps) {
    printf("0x%lx\t\t%s\t\t%d\n", bp.address, (bp.enabled ? "yes" : "no"),
           bp.counter);
  }
}

void ptrace_wrapper::continue_process() {
  if (!is_child_alive) {
    printf("Error: Child process is not alive.\n");
    return;
  }
  ptrace(PTRACE_CONT, child_id, NULL, NULL);
  waitpid(child_id, &status, 0);
  if (WIFEXITED(status)) {
    is_child_alive = false;
    printf("\nChild process exited with status %d.\n", WEXITSTATUS(status));
  } else if (WIFSTOPPED(status)) {
    int sig = WSTOPSIG(status);
    if (sig == SIGINT) {
      printf("\nProcess stopped by SIGINT.\n");
    } else if (sig == SIGTRAP) {
      printf("\nProcess stopped by SIGTRAP (Breakpoint).\n");
    } else {
      printf("\nProcess stopped by signal %d.\n", sig);
    }
  }
}

void ptrace_wrapper::kill_child_process() {
  if (is_child_alive && child_id > 0) {
    printf("\nTerminating child process (PID: %d)...\n", (int)child_id);
    kill(child_id, SIGKILL);
    int exit_status;
    waitpid(child_id, &exit_status, 0);
    is_child_alive = false;
    child_id = -1;
  }
}

void ptrace_wrapper::attach_to_process(pid_t pid) {
  child_id = pid;

  if (ptrace(PTRACE_ATTACH, child_id, NULL, NULL) == -1) {
    perror("ptrace ATTACH failed");
    is_loaded = false;
    is_child_alive = false;
    child_id = -1;
    return;
  }

  int status;
  waitpid(child_id, &status, 0);
  is_loaded = true;
  is_child_alive = true;
  printf("\nSuccessfully attached to process %d.\n", child_id);
}

bool ptrace_wrapper::is_loaded_status() const { return is_loaded; }
