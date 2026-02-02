# Debugger Test Suite

This document details the test suite for the Minimal Debugger. The tests verify core functionalities: breakpoints, register inspection, single-stepping, and process continuation.

## Test Target: `dummy/simple`

The target is an assembly program (`dummy/simple.s`) compiled to an ELF binary.

**Logic:**
1.  `mov $0x12345678, %rax` (Load distinct value)
2.  `nop` (Address `0x401007` - **Breakpoint Target**)
3.  `mov $60, %rax` (Exit syscall)
4.  `xor %rdi, %rdi`
5.  `syscall`

**Key Addresses (from `objdump -d dummy/simple`):**
*   `0x401007`: `nop` (Breakpoint)
*   `0x401008`: `mov $0x3c, %rax` (Instruction following `nop`)

## Test Execution Results

Tests are run via `make input_tests`.

### 1. Basic Breakpoint & Continue (`input_1.txt`)
*   **Action:** Load `dummy/simple`, Break `0x401007`, Run.
*   **Result:**
    *   Stops with `RIP: 0x401008`. This is expected x86 behavior (trap interrupt pushes IP to next instruction).
    *   `RAX: 0x12345678`. Correctly reflects the previous instruction.
*   **Action:** Continue.
*   **Result:** Process exits with status 0.
*   **Status:** **PASS**

### 2. Single Stepping (`input_2.txt`)
*   **Action:** Break `0x401007`, Run.
*   **Result:** Stops at `0x401008` (post-trap).
*   **Action:** Step (`s`).
*   **Result:**
    *   Debugger output: "single step ran successfully".
    *   `RIP`: `0x401008`.
    *   **Analysis:** The debugger correctly identified it was at a breakpoint. It rewound `RIP` to `0x401007`, restored the `nop` instruction, single-stepped it, and re-inserted the breakpoint. Since `nop` is 1 byte, `RIP` advanced from `0x401007` to `0x401008`. The visible state "didn't change" because we stepped a 1-byte instruction after a 1-byte trap offset, but the logic executed correctly.
*   **Action:** Continue.
*   **Result:** Process exits with status 0.
*   **Status:** **PASS**

### 3. Constants Check (`input_test_const.txt`)
*   **Action:** Break `0x401007`, Run, Check Regs.
*   **Result:** `RAX` is `0x12345678`.
*   **Status:** **PASS**

### 4. Basic Status (`input.txt`)
*   **Action:** Load, Status (`ps`), Exit.
*   **Result:**
    *   `ps` reports a valid Child PID and "Process Status: Running".
    *   **Note:** The debugger `load` command immediately forks the child process (stopped at entry), which explains why a PID exists before `run` is explicitly called.
*   **Status:** **PASS**

## Verification Summary
All automated tests passed. The debugger correctly handles:
- Setting software breakpoints (`int3`).
- Transparently stepping over breakpoints (rewind -> restore -> step -> re-break).
- Register retrieval via `ptrace`.
- Process lifecycle management (fork, wait, exit).