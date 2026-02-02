# Bareshell Technical Documentation

## 1. Project Overview

Bareshell is a lightweight, modular command-line interpreter (shell) written in C. The project is designed for simplicity, focusing on core shell functionalities without the overhead of full-featured shells.

### Core Features

*   **Command Execution:** Capable of executing simple commands with arguments.
*   **Argument Parsing:** Robust handling of quoted arguments (single and double quotes).
*   **I/O Redirection:** Support for standard input (`<`) and standard output (`>`) redirection.
*   **Pipes:** Implementation of single-level pipes (`|`) to chain the output of one command to the input of another.
*   **Background Execution:** Ability to run commands in the background using the `&` operator.
*   **Built-in Commands:** Native support for navigation (`cd`) and termination (`exit`).

## 2. Architecture

The application is built on a modular, decoupled architecture centered around a Read-Eval-Print Loop (REPL).

### Control Flow

1.  **Initialization:** The `main` function initializes the environment and triggers the REPL.
2.  **Prompt:** The REPL (`looper`) displays a prompt and awaits user input.
3.  **Input:** The Input (`multiline_input`) module captures the raw command string from `stdin`.
4.  **Parsing:** The Parser (`parse_to_command`) tokenizes the raw string and converts it into a `struct Command` linked list.
5.  **Execution:** The Execution Engine (`execute_commands`) iterates through the linked list. It manages process creation (`fork`), wiring of pipes and redirections (`dup2`), and command execution (`execve`).
6.  **Loop:** Steps 2–5 repeat until the user issues the `exit` command.

## 3. Module Descriptions

### 3.1. Main

*   **Source:** `src/main.c`
*   **Purpose:** Entry point of the application.
*   **Functionality:** Initializes the shell's environment and initiates the main Read-Eval-Print Loop (REPL) by repeatedly calling the `looper()` function. It also handles `SIGINT` signals (Ctrl-C) to allow the shell to continue running after an interruption, restarting the REPL cycle.

### 3.2. REPL (Read-Eval-Print Loop)

*   **Source:** `src/repl/looper.h`, `src/repl/looper.c`
*   **Key Function:** `looper()`
*   **Purpose:** Orchestrates the shell's lifecycle, managing the continuous cycle of reading user input, parsing it, executing the commands, and handling errors.
*   **Responsibilities:**
    *   Displays the command prompt (`>>` for primary input, `.  ` for multiline input).
    *   Retrieves user input via the `multiline_input` function from the Input module.
    *   Handles `SIGINT` by setting a flag and returning a specific error code to `main` for clean restart.
    *   Delegates raw input processing to the Parser module (`parse_to_command`).
    *   Interprets `parse_error` codes and prints user-friendly error messages for issues like unpaired quotes, missing commands, or syntax errors.
    *   Passes successfully parsed structured commands to the Execution Engine (`execute_commands`).
    *   Handles memory cleanup for command structures post-execution.

### 3.3. Parser

*   **Source:** `src/parser/command.h`, `src/parser/command.c`
*   **Key Function:** `parse_to_command()`
*   **Purpose:** Transforms raw string input from the user into a structured, executable format (`struct Command` linked list). It's responsible for understanding the syntax of commands, arguments, redirections, and pipes.
*   **Responsibilities:**
    *   **Tokenization:** Breaks down the input string into individual tokens, recognizing spaces, quotes, and special characters (`|`, `>`, `<`, `&`) as delimiters or tokens themselves. The `get_token` helper function handles this.
    *   **Quote Handling:** Correctly processes single and double-quoted segments, preserving internal whitespace and allowing escaped quotes, and identifying unpaired quotes as an error.
    *   **Struct Population:** Populates `struct Command` nodes with parsed data, including the program name, arguments, input/output files for redirection, and background execution flags. The `parse_command_helper` function assists in this.
    *   **Chaining:** Links multiple `Command` structs together via the `next_command` pointer to represent pipelines (e.g., `ls | grep foo`), creating a chain of commands for the Execution Engine.
    *   **Error Detection:** Identifies and reports various parsing errors, such as missing commands, duplicate redirections, or general syntax errors.

### 3.4. Execution Engine

*   **Source:** `src/utils.h`, `src/utils.c`
*   **Key Function:** `execute_commands()`
*   **Purpose:** Executes the parsed command structures, managing processes, I/O, and job control aspects of the shell.
*   **Responsibilities:**
    *   **Built-ins:** Directly handles `cd` and `exit` commands within the parent shell process, preventing the need for `fork` and `exec` for these internal commands.
    *   **External Commands:** For non-built-in commands, it uses `fork()` to spawn child processes for executing system binaries.
    *   **Path Resolution:** Employs the `resolve_program_path()` function to search the directories specified in the `PATH` environment variable, as well as the current directory, to locate the full path of the executable program.
    *   **Piping:** Manages the creation of pipes (`pipe()`) and connects the `stdout` of one command to the `stdin` of the next command in a pipeline using `dup2()`.
    *   **Redirection:** Handles input (`<`) and output (`>`) redirection by opening the specified files and using `dup2()` to connect them to the child process's `stdin` or `stdout`.
    *   **Concurrency & Job Control:** Determines if a command should run in the background (`&`) and manages process groups using `setpgid()` and `tcsetpgrp()` to control which process group is in the foreground (for interactive input). It also waits for foreground child processes using `waitpid()`.
    *   **Single Command Execution:** The `execute_single_command` helper function encapsulates the logic for setting up a child process, including signal handling, I/O redirection, and invoking `execve()` to replace the child process with the target program.

### 3.5. Input

*   **Source:** `src/io/input.h`, `src/io/input.c`
*   **Key Function:** `multiline_input()`
*   **Purpose:** Provides a robust mechanism for capturing user input from the standard input stream, including support for multiline commands.
*   **Responsibilities:**
    *   Reads from `stdin` continuously using `fgets` until a complete command (not ending with `\` and with balanced quotes) is detected or the end-of-file is reached.
    *   Manages dynamic memory allocation for the input buffer to accommodate commands of varying lengths.
    *   Handles quoted strings to correctly identify command completion, even across multiple lines.
    *   Provides visual feedback with `>>` for the first line and `.  ` for subsequent lines of a multiline input.

### 3.6. File Handling (io/filehandler)

*   **Source:** `src/io/filehandler.h`, `src/io/filehandler.c`
*   **Key Function:** `list_files()`
*   **Purpose:** Provides utilities for interacting with the file system, specifically for listing directory contents.
*   **Responsibilities:**
    *   Opens and reads a specified directory.
    *   Dynamically allocates memory to store the names of files and subdirectories found within the given path.
    *   Returns a null-terminated array of strings, where each string is the name of an entry in the directory.

## 4. Core Data Structures

### `struct Command`

Defined in `src/parser/command.h`, this structure is the backbone of the shell's operation. It represents a single command unit or a node in a pipeline.

```c
struct Command {
    int arg_count;    // Number of arguments
    char** args;      // Argument vector (e.g., {"ls", "-l", NULL})
    char* program;    // The binary name (e.g., "ls")
    char* input_file; // Filename for stdin redirection (<)
    char* output_file; // Filename for stdout redirection (>)
    bool background;  // Flag for background execution (&)
    struct Command* next; // Pointer to the next command in the pipe
};
```

## 5. Build Process

The project uses `make` for build automation. The `makefile` handles compilation of source files in `src/` and linking.

*   **Build Project:**
    `Bash: make`
    Generates the `bareshell` executable in the root directory.
*   **Clean Artifacts:**
    `Bash: make clean`
    Removes object files and the executable.

## 6. Testing

The project utilizes the `minunit.h` framework for unit testing, located in the `tests/` directory.

*   **Current Scope:** `tests/command_test.c` (Verifies parser logic).
*   **Run Tests:**
    `Bash: make test`
    Compiles and executes the test suite.
