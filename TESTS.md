# Integrated System Test Suite

This document details the automated test suite used to verify the integration of the Shell, VM, and Debugger. The tests ensure that process control, debugging commands, and memory management features work as expected.

## Running Tests

The tests are integrated into the main `Makefile`.

To run **all tests**:
```bash
make tests
```

To run a **specific test**:
```bash
make test_<name>
```
(e.g., `make test_step`)

## Test Cases

All test inputs are located in the `tests/` directory.

### 1. `test_step`
-   **File**: `tests/test_step.txt`
-   **Target**: `simple_loop.asm`
-   **Action**: Submits the program, enters debug mode, and executes `step` multiple times.
-   **Verification**: Ensures that the PC advances correctly and the debugger stays in control after each step.

### 2. `test_breakpoints`
-   **File**: `tests/test_breakpoints.txt`
-   **Action**: Sets a breakpoint (`break 2`), continues execution, and verifies that the VM stops at the correct address (`PC: 2`).
-   **Verification**: Confirms that the execution loop respects breakpoints.

### 3. `test_stack`
-   **File**: `tests/test_stack.txt`
-   **Action**: Pushes values onto the stack (via assembly execution) and inspects them using the `stack` command.
-   **Verification**: Ensures that the debugger can correctly read and display the VM's runtime stack.

### 4. `test_heap_mem`
-   **File**: `tests/test_heap_mem.txt`
-   **Target**: `heap_test.asm` (Uses `CONS` to allocate memory).
-   **Action**:
    1.  Executes `CONS` instructions to create objects.
    2.  Calls `memstat` to verify `Heap Objects` count increases.
    3.  Calls `leaks` to verify the objects are tracked.
    4.  Calls `gc` to ensure collection runs without crashing.
-   **Verification**: Validates the entire memory management pipeline (Allocation -> Tracking -> GC -> Reporting).

### 5. `test_help`
-   **File**: `tests/test_help.txt`
-   **Action**: Simply runs the `help` command in the debugger.
-   **Verification**: Basic sanity check that the REPL is responsive.

## Test Infrastructure

The tests work by piping pre-defined input scripts into the `bareshell` executable:

```bash
cat tests/test_case.txt | ./bareshell/bareshell
```

Since the `bareshell` reads from `stdin`, it processes the commands (`submit`, `debug`, etc.) and passes the subsequent lines to the VM's REPL when the debugger takes control. This allows for fully automated end-to-end verification.
