#ifndef PARSER_H
#define PARSER_H

// Splits the input string into tokens and returns a dynamically allocated array of tokens.
// It is the caller's responsibility to free the allocated memory using free_args.
char **parse_input(const char *input);

// Frees the memory allocated for an array of tokens.
void free_args(char **args);

#endif // PARSER_H
