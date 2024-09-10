#include "fileParser.h"
#include <stdlib.h>
#include <string.h>

struct lines* readFile(FILE *file, char *fileName) {
    char currentLine[100];
    struct lines *allLines = malloc(sizeof(struct lines));
    if (allLines == NULL) {
        perror("Unable to allocate memory\n");
        return NULL;
    }
    allLines->tokens = (struct stringArray*) malloc(DEFAULT_AMOUNT * sizeof(struct stringArray));
    allLines->allocatedAmount = DEFAULT_AMOUNT;
    allLines->numTokenLines = 0;
    allLines->numLines = 0;
    if (allLines->tokens == NULL) {
        perror("Unable to allocate memory\n");
        free(allLines);
        return NULL;
    }
    int lineNumber = 0;
     while (fgets(currentLine, 100, file) != NULL) {
        if (strlen(currentLine) == 0) {
            perror("No contents in file ");
            perror(fileName);
            perror("\r\n");
        }
        // Check if new line
        if (currentLine[0] == '\n') {
            break;  // Done reading in file
        }
        //allLines->tokens[lineNumber].type = (4 * sizeof(int) * DEFAULT_AMOUNT);
        // Check if need more memory
        //printf("Allocated amount: %d\n", allLines->allocatedAmount);
        if (allLines->numTokenLines >= allLines->allocatedAmount) {
            //printf("%d | ", allLines->numTokenLines);
            allLines->allocatedAmount = allLines->numTokenLines * 2;
            //printf("%d\n", allLines->allocatedAmount);
            allLines->tokens = realloc(allLines->tokens, allLines->allocatedAmount * sizeof(struct stringArray));
            if (allLines->tokens == NULL) {
                perror("Unable to allocate additional memory\n");
                return NULL;
            }
        }

        // Tokenize
        
        char *token = strtok(currentLine, ", \t \n");
        int tokenID = 0;
        allLines->tokens[lineNumber].numStrings = 0;
        allLines->tokens[lineNumber].stringArray = (char **) malloc(sizeof(char *) * DEFAULT_AMOUNT);
        allLines->tokens[lineNumber].allocatedAmount = DEFAULT_AMOUNT;
        if (token[0] == '#') {
            token = NULL;
        } else {
            while (token != NULL) {
                if (token[0] == '#' || token[0] == '\n') {
                    token = NULL;
                    break;
                }
                if (allLines->tokens[lineNumber].numStrings >= allLines->tokens[lineNumber].allocatedAmount) {
                    allLines->tokens[lineNumber].stringArray = realloc(allLines->tokens[lineNumber].stringArray, sizeof(char *) * allLines->tokens[lineNumber].numStrings * 2);
                    if (allLines->tokens[lineNumber].stringArray == NULL) {
                        perror("Unable to allocate additional memory\n");
                        return NULL;
                    }
                    allLines->tokens[lineNumber].allocatedAmount = allLines->tokens[lineNumber].numStrings * 2;
                }
                allLines->tokens[lineNumber].stringArray[tokenID] = (char *) malloc(strlen(token) + 1);
                if (allLines->tokens[lineNumber].stringArray[tokenID] == NULL) {
                    perror("Unable to allocate additional memory\n");
                    return NULL;
                }
                strcpy(allLines->tokens[lineNumber].stringArray[tokenID], token);
                allLines->tokens[lineNumber].numStrings++;
                tokenID++;
                token = strtok(NULL, ", \t \n");
            }
            lineNumber++;
            allLines->numTokenLines++;
        }
        allLines->numLines++;
    }
    fclose(file);
    return allLines;
}

