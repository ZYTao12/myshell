/*
 * executor.c
 *
 * Contains functions for executing shell commands.
 * This implementation handles commands with no arguments and with arguments.
 *
 * The execute_command function forks a child process and uses execvp
 * to run the command. The parent process waits for the child to complete.
 * In the child process, before executing execvp, it sets up input/output
 * redirection if specified using "<" and ">" operators.
 *
 * The code is written with the assistance of generative AI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> 
#include "executor.h"

/*
 * setup_redirection:
 *
 * Processes the tokenized command to set up input and output redirection.
 * It scans the tokens for redirection operators ("<" and ">") and opens
 * the corresponding files. Then, dup2() is used to redirect STDIN or STDOUT
 * accordingly.
 *
 * This function returns a new arguments array with the redirection tokens
 * (and the filenames following them) removed.
 *
 * Parameters:
 *   args - The original NULL-terminated tokenized command array.
 *
 * Returns:
 *   A new arguments array with redirection tokens removed.
 *   Exits the process if any errors occur.
 */
char **setup_redirection(char **args) {
    int count = 0;
    // First pass: count tokens that are not redirection operators
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0) {
            i++; // skip the file token as well
            continue;
        }
        count++;
    }
    
    // Allocate new arguments array
    char **new_args = malloc((count + 1) * sizeof(char *));
    if (!new_args) {
        perror("myshell");
        exit(EXIT_FAILURE);
    }
    
    int j = 0;
    // Second pass: set up redirection and copy non-redirection tokens
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            i++; // Next token should be the output file
            if (args[i] == NULL) {
                fprintf(stderr, "myshell: syntax error: expected output file after '>'\n");
                exit(EXIT_FAILURE);
            }
            int fd = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            close(fd);
        } else if (strcmp(args[i], "<") == 0) {
            i++; // Next token should be the input file
            if (args[i] == NULL) {
                fprintf(stderr, "myshell: syntax error: expected input file after '<'\n");
                exit(EXIT_FAILURE);
            }
            int fd = open(args[i], O_RDONLY);
            if (fd < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            close(fd);
        } else {
            new_args[j++] = args[i];
        }
    }
    new_args[j] = NULL;
    return new_args;
}

/*
 * execute_pipeline:
 *
 * Executes a sequence of commands connected by pipes.
 * It splits the tokenized command (args) into individual commands using
 * the pipe symbol "|" as the delimiter, creates the necessary pipes, forks
 * processes for each command segment, and connects their input/output accordingly.
 *
 * The shell waits for all processes in the pipeline to finish before returning.
 */
void execute_pipeline(char **args) {
    // Count the number of pipes in the command.
    int num_pipes = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0)
            num_pipes++;
    }
    int num_commands = num_pipes + 1;
    
    // Allocate an array for each command segment.
    char ***commands = malloc(num_commands * sizeof(char **));
    if (!commands) {
        perror("myshell");
        exit(EXIT_FAILURE);
    }
    
    int cmd_index = 0, start = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            int length = i - start;
            commands[cmd_index] = malloc((length + 1) * sizeof(char *));
            if (!commands[cmd_index]) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            for (int j = 0; j < length; j++)
                commands[cmd_index][j] = args[start + j];
            commands[cmd_index][length] = NULL;
            cmd_index++;
            start = i + 1;
        }
    }
    // Last command segment.
    int length = 0;
    for (int i = start; args[i] != NULL; i++)
        length++;
    commands[cmd_index] = malloc((length + 1) * sizeof(char *));
    if (!commands[cmd_index]) {
        perror("myshell");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < length; j++)
        commands[cmd_index][j] = args[start + j];
    commands[cmd_index][length] = NULL;
    
    // Create the required pipes. Each pipe requires 2 file descriptors.
    int pipefd[2 * num_pipes];
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefd + i * 2) < 0) {
            perror("myshell");
            exit(EXIT_FAILURE);
        }
    }
    
    // Fork processes for each command segment.
    int pid, status;
    for (int i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid < 0) {
            perror("myshell");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // If not the first command, redirect STDIN to the previous pipe's read end.
            if (i != 0) {
                if (dup2(pipefd[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("myshell");
                    exit(EXIT_FAILURE);
                }
            }
            // If not the last command, redirect STDOUT to the current pipe's write end.
            if (i != num_commands - 1) {
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("myshell");
                    exit(EXIT_FAILURE);
                }
            }
            // Close all pipe file descriptors in the child.
            for (int j = 0; j < 2 * num_pipes; j++)
                close(pipefd[j]);
            
            // Handle any redirection within this command segment.
            char **cmd_with_redir = setup_redirection(commands[i]);
            if (execvp(cmd_with_redir[0], cmd_with_redir) == -1) {
                if (errno == ENOENT)
                    fprintf(stderr, "myshell: command not found: %s\n", cmd_with_redir[0]);
                else
                    fprintf(stderr, "myshell: %s: %s\n", cmd_with_redir[0], strerror(errno));
                free(cmd_with_redir);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Parent: close all pipe file descriptors.
    for (int i = 0; i < 2 * num_pipes; i++)
        close(pipefd[i]);
    
    // Wait for all child processes in the pipeline.
    for (int i = 0; i < num_commands; i++)
        wait(&status);
    
    // Free allocated command segments.
    for (int i = 0; i < num_commands; i++)
        free(commands[i]);
    free(commands);
}

/*
 * execute_command:
 *
 * Executes a command with its arguments, handling redirection and pipelines.
 * If a pipe symbol ("|") is found in the token list, the command is processed
 * as a pipeline. Otherwise, the command is executed as a single process.
 * Single process handling forks a new process. In the child process, it calls setup_redirection()
 * to configure any requested redirection, then uses execvp() to execute the command.
 * If execvp() fails, it prints an error message. The parent process waits for the child.
 *
 * Parameters:
 *   args - A NULL-terminated array of strings, where the first element is the command
 *          and subsequent elements are its arguments (which may include redirection tokens).
 */
void execute_command(char **args) {
    // Check if the command contains a pipe symbol.
    int has_pipe = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            has_pipe = 1;
            break;
        }
    }
    if (has_pipe) {
        execute_pipeline(args);
        return;
    }
    
    // No pipe found: handle as a single command.
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid < 0) {
        // Error occurred while forking
        perror("myshell");
    } else if (pid == 0) {
        // Child process: 
        // setp up redirection if any
        char **new_args = setup_redirection(args);
        // execute the command using execvp
        if (execvp(new_args[0], new_args) == -1) {
            if (errno == ENOENT)
                fprintf(stderr, "myshell: command not found: %s\n", new_args[0]);
            else
                fprintf(stderr, "myshell: %s: %s\n", new_args[0], strerror(errno));
            free(new_args);
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process: wait for the child process to complete
        while (waitpid(pid, &status, WUNTRACED) > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status))
                break;
        }
    }
}