# Unix-Based Shell (C++)

## Introduction

This project is a custom Unix-based shell implemented in C++. It provides an interactive command-line interface that supports command execution, built-in commands, history management, auto-completion, and tokenization.

The shell leverages GNU Readline for input handling and command history, and uses standard Unix system calls (`fork`, `exec`, `wait`, etc.) to execute external programs.

---

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Installation](#installation)
- [Usage](#usage)
- [Architecture Overview](#architecture-overview)
- [Dependencies](#dependencies)
- [Contributors](#contributors)

---

## Features

- Interactive shell prompt
- Execution of external Unix commands
- Built-in commands:
  - `echo`
  - `type`
  - `exit`
  - `pwd`
  - `cd`
  - `history`
- Command history persistence via `HISTFILE`
- Tab auto-completion
- Command tokenization and parsing
- Process management using `fork()` and `exec()`
- Redirection support (if implemented in executor/tokenizer)
- Modular architecture for maintainability

---

## Project Structure
```code
Rshell/
│
├── src/
│   ├── main.cpp
│   ├── tokenizer.cpp / tokenizer.h
│   ├── completion.cpp / completion.h
│   ├── history.cpp / history.h
│   ├── executor.cpp / executor.h
│   ├── builtins.cpp / builtins.h
│
├── Makefile
└── README.md
```
---

## Installation

### Prerequisites

- C++17 or later
- GNU Readline library
- Unix-based operating system (Linux/macOS)

### Install Readline (if not installed)

On Ubuntu/Debian:
```bash
sudo apt install libreadline-dev
```
On macOS:
```bash
brew install readline
```
Compile
```bash
g++ -std=c++17 main.cpp tokenizer.cpp executor.cpp builtins.cpp completion.cpp history.cpp -lreadline -o mysh
```
## Usage

Run the Shell:
```bash
./mysh
```
You will see a prompt:
```code
$
```
To exit:
```code
exit
```
---

## Architecture-Overview

This project implements a lightweight, modular Unix-like shell.  
The design is split into several components—each responsible for a distinct part of shell functionality.

---

## 🏁 1. `main.cpp`

The entry point of the shell.

### Responsibilities
- Initializes the shell environment
- Loads command history from `$HISTFILE`
- Configures GNU Readline (input handling, tab completion)
- Runs the main REPL loop:
  - Display prompt
  - Read user input
  - Tokenize, parse, and execute commands

---

## 🔤 2. Tokenizer (`tokenize.cpp`)

Responsible for breaking input lines into meaningful units.

### Responsibilities
- Parse raw user input
- Split commands into tokens (arguments, operators, separators)
- Build token structures for the executor
- Handle quotes, escapes, special characters, etc.

---

## ⚙️ 3. Executor (`executor.cpp`)

Runs commands provided by the tokenizer.

### Responsibilities
- Manage **process creation** using:
  - `fork()`
  - `execvp()`
  - `waitpid()`
- Handle I/O behavior and command flow
- Manage foreground process execution
- Integrates built-in command checks before spawning children

---

## 📦 4. Built-ins (`builtins.cpp`)

Implements commands handled directly by the shell without spawning new processes.

### Responsibilities
- Execute internal commands such as:
  - `cd`
  - `exit`
  - `history`
  - (Add more as needed)
- Improve performance by avoiding `fork()`

---

## 🔁 5. Completion (`completion.cpp`)

Handles custom auto-completion using GNU Readline.

### Responsibilities
- Provide file and command name suggestions
- Integrate with Readline’s completion API
- Support context-aware completions

---

## 🕒 6. History (`history.cpp`)

Manages persistent shell history.

### Responsibilities
- Load history at startup from `$HISTFILE`
- Append new entries to history
- Save on exit
- Ensure cross-session persistence

---

## Dependencies

This project relies on standard C++17 features along with Unix system calls and the GNU Readline library.  
The following dependencies are required to build and run the shell:

### 🧩 Core Language & Libraries
- **C++17** or later  
- **GNU Readline** (for interactive input and history)
- **POSIX/Unix APIs**:
  - `fork()`, `execv()`, `waitpid()`
  - `dup2()`, `open()`
  - `access()`
  - `getcwd()`
- **C++ Standard Libraries**:
  - `<iostream>`  
  - `<string>`  
  - `<sstream>`  
  - `<vector>`  
  - `<set>`  
  - `<algorithm>`  
  - `<fstream>`  
  - `<filesystem>`  
  - `<limits.h>`  
  - `<unistd.h>`  
  - `<sys/wait.h>`  
  - `<fcntl.h>`  

### 📦 External Dependency: GNU Readline
Used for:
- Command-line editing  
- History navigation  
- Tab auto-completion  

## Contributors

- **Rishav Kalani** — Creator and sole contributor of the project.
