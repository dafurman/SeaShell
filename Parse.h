#ifndef Parse_h
#define Parse_h

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#endif /* Parse_h */

/**
 * A structure used to contain a logical parsing of a user's input.
 */
typedef struct lineInput {
    int numPipes, numTokens;
    char *tokens[80];
    int redirectedInputIndex, redirectedOutputIndex;
    int pipeIndices[80];
} LineInput;

/**
 * Parse a given line by tokenizing it and placings its components into a LineInput structure.
 * @param line The line that the user inputted, which is to be parsed.
 * @param lineInput A structure containing the logical parsing of the line, which is filled by this function.
 * @return 0 if success, a non-zero integer otherwise.
 */
int parse(char *line, LineInput *lineInput);

/**
 * All this does is remove '\' characters
 * @param input The input to remove '\' from
 */
void stripEscapeChars(LineInput *input);

/**
 * If the command is followed by quotes, then convert everything in the quotes
 * into a single string argument that is used with the command.
 * @param input The input to check and modify if quotes are found.
 */
void processQuotes(LineInput *input);

/**
 * Print a given line input. Print the number of commands followed by each token that is parsed.
 * @param lineInput The structure of parsed tokens to be printed.
 */
void printLineInput(LineInput *lineInput);
