#ifndef Execute_h
#define Execute_h

#include "Parse.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>

#endif /* Execute_h */

/**
 * Process a 'cd' command
 * @param cmd The 'cd' command to process
 */
void processChangeDirectory(char *cmd[]);

/**
 * Process the echo command separately, to only print what follows it.
 * @param cmd The command to process
 */
void processEcho(char*cmd[]);

/**
 * Used to process a single command
 * @param cmd The command to process, including any arguments it may have.
 */
void processSingleCommand(char *cmd[]);

/**
 * This function checks to see if the cmd matches any built-in commands
 * and simply runs them if it finds any matches.
 * @return Whether or not the command matched a built-in command.
 */
bool processBuiltInCmd(char *cmd[LINE_MAX]);

/**
 * This function takes care of any input redirection that may occur.
 * Either sendingFile or sendingCmd must be passed (Exclusive or)
 *
 * @param sendingFile The file to be read from.
 */
void handleInputRedirection(char sendingFile[]);

/**
 * This function takes care of any output redirection that may occur.
 * @param receivingFile the file that should outputted to.
 */
void handleOutputRedirection(char receivingFile[]);

/*
 * Execute a line of input based on the user's input.
 * @param lineInput The structure representing a user's input into the shell.
 */
void execute(LineInput *input);
