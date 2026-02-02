# Debugger Loop Test Suite

This document details the loop-specific tests for the Minimal Debugger. These tests verify breakpoint handling within loops and single-stepping through branching logic.

## Test Target: `dummy/loop`

The target is an assembly program (`dummy/loop.s`) compiled to an ELF binary.

**Logic:**
1.  `xor %rcx, %rcx` (Initialize counter to 0)
2.  `inc %rcx` (Address `0x401003` - Loop Start)
3.  `cmp $5, %rcx` (Address `0x401006`)
4.  `jne loop_start` (Jump back if not equal)
5.  Exit syscall

**Key Addresses (from `objdump -d dummy/loop`):**
*   `0x401003`: `inc %rcx`
*   `0x401006`: `cmp $0x5, %rcx`

## Test Execution Results

Tests are run via `make tests_loop`.

### 1. Loop Iteration & Breakpoints (`input_loop.txt`)
*   **Action:** Load `dummy/loop`, Break `0x401006` (the `cmp` instruction), Run.
*   **Result 1 (1st Hit):**
    *   Stops at `0x401006` (before `cmp` executed, but `inc` has executed).
    *   `RCX`: `0x1` (1st iteration).
*   **Action:** Continue.
*   **Result 2 (2nd Hit):**
    *   `RCX`: `0x2` (2nd iteration).
*   **Action:** Continue x3.
*   **Results (3rd, 4th, 5th Hits):**
    *   `RCX`: `0x3`, `0x4`, `0x5`.
*   **Action:** Continue.
*   **Result:** Process exits (loop condition `rcx != 5` fails, falls through to exit).
*   **Status:** **EXPECTED PASS**

### 2. Stepping Through Loop Logic (`input_loop_step.txt`)
*   **Action:** Load `dummy/loop`, Break `0x401003` (Start of loop), Run.
*   **Result:** Stops at `0x401003`. `RCX` is `0` (before `inc`).
*   **Action:** Step (`s`).
*   **Result:** Executes `inc %rcx`. Stops at `0x401006`. `RCX` is `1`.
*   **Action:** Step (`s`).
*   **Result:** Executes `cmp`. Stops at `0x40100a` (the `jne`). Flags set.
*   **Action:** Step (`s`).
*   **Result:** Executes `jne`. Jumps back to `0x401003`.
*   **Action:** Continue.
*   **Result:** Process runs through remaining iterations and exits.
*   **Status:** **EXPECTED PASS**
