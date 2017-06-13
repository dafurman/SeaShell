#include "Execute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <signal.h>

/* 
 * If a signal interrupt is detected (Ctrl+C), this function will be called instead of terminating the program.
 */
void ignoreCtrlC(int sig_num) {
    printf("\b \b\b \b\n? "); // Remove "^C" from the console screen so that it never shows up.
    fflush(stdout);
}

void processLine(char *line, FILE *input) {
    LineInput emptyLineInput = {0};
    
    if(line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = '\0';   /* zap the newline */
    
    if (line[0] != '#') {
        LineInput *lineInput = &emptyLineInput;
        parse(line, lineInput);
//        printLineInput(lineInput);
        execute(lineInput);
    }
    if(input == stdin)
    {
        printf("? ");
        fflush(stdout);
    }
}


int main(int argc, char *argv[]) {
    FILE *input = NULL;
    char line[LINE_MAX] = "";
    
    if(argc == 2)
    {
        input = fopen(argv[1], "r");
        if(input == NULL)
        {
            processLine(argv[1], input);
            exit(1);
        }
    } else if (argc == 1) {
        input = stdin;
        
        printf("? ");
        /* By default, printf will not "flush" the output buffer until
         * a newline appears.  Since the prompt does not contain a newline
         * at the end, we have to explicitly flush the output buffer using
         * fflush.
         */
        fflush(stdout);
    } else {
        int i;
        for (i = 1; i < argc; i++) {
            strcat(line, argv[i]);
            strcat(line, " ");;
        }
        // Check to see if any of the arguments should be grouped together
        for (i = 0; argv[i]; i++) {
            int j;
            bool spaceFound = false;
            for (j = 0; argv[i][j]; j++) {
                if (argv[i][j] == ' ') {
                    spaceFound = true;
                    break;
                }
            }
            if (spaceFound) {
                char buffer[LINE_MAX];
                sprintf(buffer, "%s \"%s\"", argv[1], argv[2]); // surround in quotes
                processLine(buffer, input);
                return 0;
            }
        }
        processLine(line, input);
        return 0;
    }
    
    // Ignore Ctrl+C
    if (signal(SIGINT, ignoreCtrlC) == SIG_ERR) {
        perror("Signal failed.\n\r");
        exit(EXIT_FAILURE);
    }
    
    setlinebuf(input);
    while(fgets(line, sizeof(line), input)) {
        processLine(line, input);
    }
    return 0;
}
