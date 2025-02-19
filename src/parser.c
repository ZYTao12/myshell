/*
 * parser.c
 *
 * Contains functions for parsing user input into tokens.
 * This implementation splits the input string based on whitespace,
 * handling both commands with no arguments and with arguments.
 *
 * The code is written with the assistance of generative AI.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define TOKEN_DELIM " \t\r\n\a"
#define INITIAL_TOKENS_SIZE 64

/*
 * parse_input: Splits the input string into tokens based on whitespace.
 *
 * Parameters:
 *   input - a string containing the user's input.
 *
 * Returns:
 *   A dynamically allocated array of strings (tokens), terminated by NULL.
 *   It is the caller's responsibility to free the memory using free_args.
 */
char **parse_input(const char *input) {
    int tokens_size = INITIAL_TOKENS_SIZE;
    int position = 0;
    char **tokens = malloc(tokens_size * sizeof(char*));
    if (!tokens) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    // Duplicate input string because strtok modifies the string
    char *input_copy = strdup(input);
    if (!input_copy) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    // Tokenize the input string using whitespace as the delimiter
    char *token = strtok(input_copy, TOKEN_DELIM);
    while (token != NULL) {
        // Duplicate each token to store in our tokens array
        tokens[position] = strdup(token);
        if (!tokens[position]) {
            fprintf(stderr, "myshell: allocation error\n");
            exit(EXIT_FAILURE);
        }
        position++;

        // If the tokens array is exceeded, reallocate to provide more space
        if (position >= tokens_size) {
            tokens_size += INITIAL_TOKENS_SIZE;
            tokens = realloc(tokens, tokens_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "myshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL; // Null-terminate the tokens array

    free(input_copy); // Free the duplicate copy of the input
    return tokens;
}

/*
 * free_args: Frees the memory allocated for the tokens array.
 *
 * Parameters:
 *   args - the array of tokens to be freed.
 */
void free_args(char **args) {
    if (args == NULL) {
        return;
    }
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}
