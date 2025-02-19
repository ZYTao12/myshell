/*
 * executor.c
 *
 * Contains functions for executing shell commands.
 * This implementation handles commands with no arguments and with arguments.
 *
 * The execute_command function forks a child process and uses execvp
 * to run the command. The parent process waits for the child to complete.
 *
 * The code is written with the assistance of generative AI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "executor.h"

/*
 * execute_command: Executes a command with its arguments.
 *
 * Parameters:
 *   args - a NULL-terminated array of strings, where the first element is the command
 *          and subsequent elements are the command arguments.
 *
 * The function forks a new process. In the child process, execvp is used to execute
 * the command. If execvp fails, an error message is printed. The parent process waits
 * for the child to finish before returning.
 */
void execute_command(char **args) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid < 0) {
        // Error occurred while forking
        perror("myshell");
    } else if (pid == 0) {
        // Child process: execute the command using execvp
        if (execvp(args[0], args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else {
        // Parent process: wait for the child process to complete
        while (waitpid(pid, &status, WUNTRACED) > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status))
                break;
        }
    }
}