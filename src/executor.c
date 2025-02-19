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
 * execute_command:
 *
 * Executes a command with its arguments, handling input/output redirection.
 * This function forks a new process. In the child process, it calls setup_redirection()
 * to configure any requested redirection, then uses execvp() to execute the command.
 * If execvp() fails, it prints an error message. The parent process waits for the child.
 *
 * Parameters:
 *   args - A NULL-terminated array of strings, where the first element is the command
 *          and subsequent elements are its arguments (which may include redirection tokens).
 */
void execute_command(char **args) {
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