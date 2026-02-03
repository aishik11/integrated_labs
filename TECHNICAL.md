# Integrated Framework Technical Documentation

## 1. System Architecture

The system follows a modular architecture where the Shell acts as the orchestrator and the VM as the execution engine.

### 1.1 Process Management (Shell)
The `bareshell` maintains a **Job Table** to track the state of submitted programs.

-   **Submission (`submit`)**:
    1.  The shell calls the Assembler to compile the `.asm` file.
    2.  It `fork()`s a new child process.
    3.  The child raises `SIGSTOP` immediately to enter a suspended state.
    4.  The child `exec()`s the `bvm` (Bytecode VM).
    5.  The shell records the PID and marks job status as `SUBMITTED`.

-   **Execution (`run`)**:
    1.  The shell sends `SIGCONT` to the target PID.
    2.  It uses `tcsetpgrp` to give the child terminal control.
    3.  It calls `waitpid` to wait for the child to finish or stop (via `Ctrl+Z`).

-   **Debugging (`debug`)**:
    1.  The shell sends `SIGCONT` to resume the process (if stopped).
    2.  It sends `SIGUSR1` to the VM process.
    3.  The VM signal handler sets a `debug_mode` flag, triggering the internal Debug REPL.

### 1.2 Bytecode Virtual Machine (VM)
The VM is a stack-based machine with a Harvard architecture (separate program and data memory).

-   **Execution Loop**:
    The main `run()` loop fetches instructions pointed to by `PC` (Program Counter) and executes them.
    Before each step, it checks:
    1.  **Debug Mode**: If set (via signal or breakpoint), it invokes `repl()`.
    2.  **Breakpoints**: If `PC` matches a breakpoint, it enables debug mode.

-   **Signal Handling**:
    The VM installs a handler for `SIGUSR1`. When received, it sets `debug_mode = true`. This allows the shell to asynchronously interrupt execution and drop the user into the debugger.

## 2. Memory Management System

### 2.1 Object Model
The VM supports tagged objects to distinguish between primitive values (Integers) and Heap Objects (Pairs, Closures).
-   **Stack**: Stores primitives and pointers to heap objects.
-   **Heap**: Linked list of allocated `Object` structs.

### 2.2 Garbage Collection (GC)
The system uses a **Mark-and-Sweep** collector.

1.  **Mark Phase**:
    -   Starts from the **Roots**: The VM Stack.
    -   Traverses all reachable objects (recursively following pointers in Pairs/Closures).
    -   Sets the `marked` bit on reached objects.

2.  **Sweep Phase**:
    -   Iterates through the entire Heap list.
    -   If an object is NOT marked, it is `free()`d and removed from the list.
    -   If marked, the bit is cleared for the next cycle.

### 2.3 `CONS` Implementation
To verify heap operations, the `CONS` (0x50) opcode was implemented.
-   **Logic**: Pops two values (Tail, Head) from the stack.
-   **Allocation**: Allocates a new `OBJ_PAIR` on the heap.
-   **Result**: Pushes the pointer to the new Pair back onto the stack.

### 2.4 Leak Detection
The VM destructor (`~VM`) runs upon process exit. It iterates through the heap list. If `num_objects > 0`, it reports a memory leak to `stderr`.

## 3. Inter-Process Communication (IPC)

The integration relies on UNIX signals for coordination:

| Signal | Sender | Receiver | Purpose |
| :--- | :--- | :--- | :--- |
| `SIGSTOP` | Self | OS | Pauses the child immediately after `fork/exec`. |
| `SIGCONT` | Shell | VM | Resumes the process for `run` or `debug`. |
| `SIGTSTP` | User | Shell/VM | Triggered by `Ctrl+Z`. Stops the running job. |
| `SIGUSR1` | Shell | VM | Triggers Debug Mode (repl) inside the VM. |

This design allows the shell to manage the VM as a standard Linux process while enabling deep introspection via the debug channel.
