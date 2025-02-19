/*
 * myshell.c
 *
 * Main file for myshell. Implements the interactive shell loop.
 * It reads user input, uses the parser to tokenize the input,
 * and then calls the executor to run the command.
 *
 * The code is written with the assistance of generative AI.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "parser.h"
#include "executor.h"

#define MAX_INPUT_SIZE 1024

int main() {
    char input[MAX_INPUT_SIZE];

    // Main shell loop
    while (1) {
        // Print shell prompt
        printf("$ ");
        fflush(stdout);

        // Read input from user (fgets returns NULL on EOF, e.g., Ctrl+D)
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove trailing newline character from input
        input[strcspn(input, "\n")] = '\0';

        // Check if user wants to exit the shell
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Check for built-in 'cd' command
        // We compare the first token to "cd"
        char *temp = strdup(input);
        if (temp == NULL) {
            fprintf(stderr, "myshell: allocation error\n");
            continue;
        }
        char *token = strtok(temp, " \t\r\n");
        if (token != NULL && strcmp(token, "cd") == 0) {
            // Get the next token, which should be the directory path
            char *dir = strtok(NULL, " \t\r\n");
            if (dir == NULL) {
                fprintf(stderr, "myshell: expected argument to \"cd\"\n");
            } else {
                if (chdir(dir) != 0) {
                    // Check errno and print a custom error message
                    if (errno == ENOENT)
                        fprintf(stderr, "cd: no such file or directory: %s\n", dir);
                    else
                        fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
                }
            }
            free(temp);
            continue;  // Skip further processing for built-in commands
        }
        free(temp);

        // Parse the input into an array of tokens (command + arguments)
        char **args = parse_input(input);
        if (args == NULL || args[0] == NULL) {
            // No valid command entered; free any allocated memory and continue
            free_args(args);
            continue;
        }

        // Execute the parsed command
        execute_command(args);
        
        // Free allocated memory for tokens
        free_args(args);
    }
    
    return 0;
}
