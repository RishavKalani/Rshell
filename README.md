# Rshell

Rshell is a simple Unix-like command-line shell written in **C++** using **POSIX system calls**.  
I built this project while working through the CodeCrafters “Build Your Own Shell” challenge to better understand how shells work internally.

## What it can do
- Run external commands using `fork()`, `execvp()`, and `waitpid()`
- Supports basic built-in commands like `cd`, `pwd`, `exit`, `echo`, and `type`
- Finds executables using the system `PATH`
- Handles shell-style quoting and argument parsing
- Supports output and error redirection (`>`, `>>`, `2>`, `2>>`)
- Tab-based autocompletion for commands using raw terminal input (`termios`)
- Basic directory navigation

## Tech used
- **C++**
- **POSIX APIs** (fork, execvp, waitpid, dup2, termios)
- Tested on Unix-like systems (Linux/macOS)

## How to run

Compile:
```bash
g++ -std=c++17 main.cpp -o rshell
```
Run:
```bash
./rshell
```

