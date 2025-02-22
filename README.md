# CS-UH 3010 myshell

A simple shell implementation in C that provides basic command-line functionality.

## Current Features

- Interactive shell prompt
- Command execution with arguments
- Basic error handling
- Memory management for command parsing
- Support for system commands through `execvp`
- Clean exit functionality

## Project Structure
```
├── Makefile
├── README.md
├── Report.pdf # To be included
└── src/
├──├── myshell.c # Main shell implementation
├──├── parser.c # Input parsing functionality
├──├── parser.h # Parser header file
├──├── executor.c # Command execution functionality
└──└── executor.h # Executor header file
```

## Implementation Details

### Main Shell Loop
The main shell implementation (in `myshell.c`) provides an interactive loop that:
- Displays a prompt (`$`)
- Reads user input
- Parses the input into commands and arguments
- Executes the commands
- Handles program termination

### Parser
The parser component:
- Splits input strings into tokens based on whitespace
- Handles dynamic memory allocation for tokens
- Provides memory cleanup functionality
- Uses standard C string manipulation functions

### Executor
The executor component:
- Creates child processes using `fork()`
- Executes commands using `execvp()`
- Handles basic error reporting
- Manages process waiting and status checking

## Building the Project

To build the project, use the provided Makefile:
```
make
```
To clean the build files:
```
make clean
```

## Running the Shell

After building, run the shell:
```
./myshell
```

## Usage

1. Start the shell
2. Enter commands at the prompt (`$`)
3. Commands will be executed with their arguments
4. To exit, type `exit` or use Ctrl+D

## Authors
Rabeya Zahan Mily

Ziyue Tao
