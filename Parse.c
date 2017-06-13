#include "Parse.h"

// An arbitrary character limit used for this assignment in lines.
const int kCharLimit = 10000;

/**
 * Parse a given line by tokenizing it and placings its components into a LineInput structure.
 * @param line The line that the user inputted, which is to be parsed.
 * @param lineInput A structure containing the logical parsing of the line, which is filled by this function.
 * @return 0 if success, a non-zero integer otherwise.
 */
int parse(char *line, LineInput *lineInput) {
    // Remove the trailing new line if one exists
    if (strchr(line, '\n')) {
        line[strcspn(line, "\n")] = 0;
    }
    printf("%s\n", line);
    
    char *tokens[kCharLimit];
    
    // to start with, assume no redirection
    lineInput->redirectedOutputIndex = -1;
    lineInput->redirectedInputIndex = -1;
    
    // Split the line into tokens.
    char *token = strtok(line, " ");
    
    int i = 0;
    for (i = 0; token != NULL; i++) {
        lineInput->tokens[i] = token;
        lineInput->numTokens++;
        tokens[i] = token;
        if (token[0] == '|' && token[1] == '\0') { // | operator
            lineInput->pipeIndices[lineInput->numPipes] = i;
            lineInput->numPipes++;
        } else if (token[0] == '<' && token[1] == '\0') { // < operator
            lineInput->redirectedInputIndex = i; // The index of the token to be redirected
            // Offset each further array element to the left, to remove this element.
            token = strtok(NULL, " ");
            continue;
        } else if (token[0] == '>' && token[1] == '\0') { // > operator
            lineInput->redirectedOutputIndex = i; // The index of the token to be redirected
            // Offset each further array element to the left, to remove this element.
            token = strtok(NULL, " ");
            continue;
            // Do nothing for part 1
        } else if (token[0] == '-' || token[0] == '\342') { // Arguments
            // Do nothing special with arguments for part 1
        }
        token = strtok(NULL, " ");
    }
    stripEscapeChars(lineInput);
    processQuotes(lineInput);
    return 0;
}

/**
 * All this does is remove '\' characters
 * @param input The input to remove '\' from
 */
void stripEscapeChars(LineInput *input) {
    int i;
    for (i = 0; input->tokens[i]; i++) {
        int j;
        for (j = i; input->tokens[i][j]; j++) {
            if (input->tokens[i][j] == '\\') { // If there's an escape character, remove the '\'
                input->tokens[i][j] = input->tokens[i][j+1];
            }
        }
    }
}

/**
 * If the command is followed by quotes, then convert everything in the quotes
 * into a single string argument that is used with the command.
 * @param input The input to check and modify if quotes are found.
 */
void processQuotes(LineInput *input) {
    if (input->tokens[1] == NULL) {
        return;
    }
    char firstArgChar = input->tokens[1][0];
    if (firstArgChar != '\'' && firstArgChar != '"') { // Check if quotes need to be processed.
        return;
    }
    input->tokens[1] = &input->tokens[1][1]; // start past the first quote
    
    char concatArg[1024] = "";
    int i;
    for (i = 1; input->tokens[i] != NULL; i++) {
        int j;
        for (j = 1; input->tokens[i][j]; j++) {
            if ((input->tokens[i][j] == '\'' || input->tokens[i][j]== '"') && input->tokens[i][j+1] == '\0') { // If this is the ending quote
                input->tokens[i][j] = '\0';
            }
        }
        strcat(concatArg, input->tokens[i]);
        if (input->tokens[i+1] != NULL) {
            strcat(concatArg, " ");
        }
    }
    input->tokens[2] = NULL;
    input->tokens[1] = concatArg;
}

/**
 * Print a given line input. Print the number of commands followed by each token that is parsed.
 * @param lineInput The structure of parsed tokens to be printed.
 */
void printLineInput(LineInput *lineInput) {
    if (lineInput->numTokens == 0) {
        printf("%d: ", 0); // Print the number of commands
    } else {
        printf("%d: ", lineInput->numPipes); // Print the number of commands
    }
    
    if (lineInput->redirectedInputIndex != -1) {
        if (lineInput->tokens[lineInput->redirectedInputIndex] != NULL) { // Print the redirected token (if it exists)
            printf("'%s' ", lineInput->tokens[lineInput->redirectedInputIndex]);
        }
    }
    
    int i;
    for (i = 0; lineInput->tokens[i]; i++) {
        if (i == lineInput->redirectedInputIndex) {
            continue; // Don't reprint a redirected token which was already printed up above.
        }
        if (i == lineInput->redirectedOutputIndex) {
            continue; // Don't print a redirected token which will be printed at the end.
        }

        if (strcmp(lineInput->tokens[i], "|") == 0) {
            printf("%s ", lineInput->tokens[i]);
        } else {
            printf("'%s' ", lineInput->tokens[i]);
        }
    }
    
    if (lineInput->redirectedOutputIndex != -1) {
        if (lineInput->tokens[lineInput->redirectedOutputIndex] != NULL) { // Print the redirected token (if it exists)
            printf("'%s' ", lineInput->tokens[lineInput->redirectedOutputIndex]);
        }
    }

    printf("\r\n");
}
