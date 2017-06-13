#include "Execute.h"

#define O_RD_WR 0600
#define O_RD 0200
#define MAX_PIPES 100


/**
 * Process a 'cd' command
 * @param cmd The 'cd' command to process
 */
void processChangeDirectory(char *cmd[]) {
    const char *homeDir = getenv("HOME");
    if (homeDir == NULL) {
        perror("Unable to access the HOME env variable.\n\r");
        exit(EXIT_FAILURE);
    }
    
    if (cmd[1] != NULL) { // If there's a directory to change to
        if (cmd[1][0] == '\'' || cmd[1][0] == '"') {
            char *fileName = &cmd[1][1];
            int i;
            for (i = 2; cmd[i] != NULL; i++) {
                strcat(fileName, " ");;
                strcat(fileName, cmd[i]);
            }
            chdir(fileName);
            return;
        }
        char *fileName = &cmd[1][0];
        chdir(fileName);
    } else {
        chdir(homeDir); // If just 'cd', go to ~
    }
}

/**
 * Process the echo command separately, to only print what follows it.
 * @param cmd The command to process
 */
void processEcho(char*cmd[]) {
    int i;
    for (i = 1; cmd[i]; i++) {
        printf("%s ", cmd[i]);
    }
    printf("\n");
    fflush(stdout);
}

/**
 * Used to process a single command
 * @param cmd The command to process, including any arguments it may have.
 */
void processSingleCommand(char *cmd[]) {
    int status;
    pid_t pid;
    switch (pid = fork()) {
        case -1:
            perror("Could not fork a child process.\n\r");
            exit(EXIT_FAILURE);
            return;
            
        case 0: // Child
            if (execvp(cmd[0], cmd) == -1) { // Execute the command w/ any arguments it has
                perror(*cmd);
                exit(EXIT_FAILURE);
            }
            return;
            
        default: // Parent
            if (waitpid(pid,&status,0) == -1) { // Wait for the child to terminate
                if (errno != EINTR) { // a signal interrupt is expected
                    perror("Unable to wait.\n\r");
                    exit(EXIT_FAILURE);
                }
            }
            return;
    }
}

/**
 * This function checks to see if the cmd matches any built-in commands
 * and simply runs them if it finds any matches.
 * @return Whether or not the command matched a built-in command.
 */
bool processBuiltInCmd(char *cmd[LINE_MAX]) {
    if (strcmp(cmd[0], "exit\0") == 0) {      // Handle exit
        exit(EXIT_SUCCESS);
        return true;
    } else if (strcmp(cmd[0], "cd\0") == 0) { // Handle cd
        processChangeDirectory(cmd);
        return true;
    }
    return false;
}

/**
 * This function takes care of any input redirection that may occur.
 * Either sendingFile or sendingCmd must be passed (Exclusive or)
 *
 * @param sendingFile The file to be read from.
 */
void handleInputRedirection(char sendingFile[]) {
    if (sendingFile) { // If input is coming from a file
        FILE *readFile;
        if ((readFile = fopen(sendingFile, "r")) == NULL) { // Open the file for reading
            perror(sendingFile);
            exit(EXIT_FAILURE);
        }
        int fileNum;
        if ((fileNum = fileno(readFile)) == -1) {
            perror(sendingFile);
            exit(EXIT_FAILURE);
        }
        if (dup2(fileNum, STDIN_FILENO) == -1) { // Have input come from the file to be read
            perror("Unable to perform duplicate a process.\n\r");
            exit(EXIT_FAILURE);
        }
        if (fclose(readFile) == -1) {
            perror("Unable to close a file descriptor.\n\r");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * This function takes care of any output redirection that may occur.
 * @param receivingFile the file that should outputted to.
 */
void handleOutputRedirection(char receivingFile[]) {
    // If output is being redirected to a following token
    if (receivingFile) {
        FILE *writeFile;
        if ((writeFile = fopen(receivingFile, "w+")) == NULL) {
            perror(receivingFile);
            exit(EXIT_FAILURE);
        }
        int fileNum;
        if ((fileNum = fileno(writeFile)) == -1) {
            perror(receivingFile);
            exit(EXIT_FAILURE);
        }
        if (dup2(fileNum, STDOUT_FILENO) == -1) { // Connect output to the file to write to
            perror("dup2() failed.\n\r");
            exit(EXIT_FAILURE);
        }
        if (fclose(writeFile) == -1) {
            perror(receivingFile);
            exit(EXIT_FAILURE);
        }
    }
}


/*
 * Execute a line of input based on the user's input.
 * The general idea is to process the input segment by segment, using pipes as deliminators.
 *
 * @param lineInput The structure representing a user's input into the shell.
 */
void execute(LineInput *input) {
    if (input->tokens[0] == NULL) {
        return;
    }
    // Check to see if the input corresponds to a built-in command, performing it if it does.
    if (processBuiltInCmd(input->tokens)) {
        return;
    }
    // Process echo differently if it's detected
    if (strcmp(input->tokens[0], "echo") == 0) {
        processEcho(input->tokens);
        return;
    }
    int numCmdsProcessed, tokenIndex = 0, inputFileDescriptor = 0;
    for (numCmdsProcessed = 0; input->tokens[tokenIndex]; numCmdsProcessed++) {
        char *tokensBeforePipe[1024];
        // clear tokensBeforePipe
        int i;
        for (i = 0; i < 1024; i++) {
            tokensBeforePipe[i] = NULL;
        }
        
        bool redirectInput = false, redirectOutput = false;
        // Determine the tokens that are in the line before the next pipe
        int numTokensBeforePipe;
        for (numTokensBeforePipe = 0; input->tokens[tokenIndex] != NULL && input->tokens[tokenIndex][0] != '|'; tokenIndex++, numTokensBeforePipe++) {
            tokensBeforePipe[numTokensBeforePipe] = input->tokens[tokenIndex];
            // Note if this series of tokens before the next pipe has redirection within it
            if (tokenIndex == input->redirectedInputIndex) {
                redirectInput = true;
            } else if (tokenIndex == input->redirectedOutputIndex) {
                redirectOutput = true;
            }
        }
        
        tokensBeforePipe[numTokensBeforePipe] = NULL; // Remove the operator and replace it with NULL to indicate the end of the command
        tokenIndex++; // Move past the pipe
        
        char *redirectedFile = NULL;
        // Iterate through each token that exists before the next pipe, or the end of the line.
        int tokenCount;
        for (tokenCount = 0; tokenCount < numTokensBeforePipe; tokenCount++) { // Iterate through each token between pipes
            const char *token = tokensBeforePipe[tokenCount];
            if ((redirectInput = token[0] == '<') || (redirectOutput = token[0] == '>')) { // If a redirection token is found
                tokensBeforePipe[tokenCount] = NULL; // Replace the operator with null to facilitate deliminating the previous command and its arguments up to this point
                redirectedFile = tokensBeforePipe[tokenCount + 1]; // The file will always follow a < or >
                break;
            }
        }
        
        // At this point, any commands and files to be
        // worked with in this section of the full line have been identified,
        // determining which is inputting, which is receiving,
        int fd[2];
        pipe(fd);
        pid_t pid;
        switch (pid = fork()) {
            case 0: // Child
                if (redirectInput) {
                    handleInputRedirection(redirectedFile);
                } else {
                    if (dup2(inputFileDescriptor, STDIN_FILENO) == -1) { // input to should be reset to STDIN
                        perror("Unable to perform duplicate a process.\n\r");
                        exit(EXIT_FAILURE);
                    }
                }
                handleOutputRedirection(redirectedFile);
                if (close(fd[0]) == -1) { // The input descriptor is not needed, as this input is just being sent to stdin
                    perror("Unable to close the stdin file descriptor.\n\r");
                    exit(EXIT_FAILURE);
                }
                
                // Execute this part of the line
                if (execvp(tokensBeforePipe[0], tokensBeforePipe) == -1) {
                    perror(tokensBeforePipe[0]);
                    exit(EXIT_FAILURE);
                }
                perror(tokensBeforePipe[0]);
                exit(EXIT_FAILURE);
                return;
            default: // Parent
                // Grab the parent's input descriptor before the child begins processing
                // so that the child can
                inputFileDescriptor = fd[0];
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    if (errno != EINTR) { // a signal interrupt is expected
                        perror("Unable to wait.\n\r");
                        exit(EXIT_FAILURE);
                    }
                }
                if (close(fd[1]) == -1) {
                    perror("Unable to close a file descriptor.\n\r");
                    exit(EXIT_FAILURE);
                }
                break;
            case -1:
                perror("Unable to fork a child process.\n\r");
                exit(EXIT_FAILURE);
                return;
        }
    }
    return;
}
