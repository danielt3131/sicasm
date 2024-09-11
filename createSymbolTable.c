#include "checker.h"
#include "tables.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

struct symbolTable* createSymbolTable(FILE *file) {
    char currentLine[100];
    struct symbolTable *symbolTable = malloc(sizeof(struct symbolTable));
    symbolTable->symbols = malloc(5 * sizeof(symbol));
    symbolTable->allocatedAmount = 5;
    int address = 0;
    int currentSymbol = 0;
    char lineSplit[25];
    int lineNumber = 0;
    bool seenStart = false;
    while (fgets(currentLine, 100, file) != NULL) {
        if (currentLine[0] != '#');
        if (!isspace(currentLine[0])) {
            struct stringArray *split = stringSplit(currentLine, "\t \n");
            if (isValidSymbol(split->stringArray[0], symbolTable) && split->numStrings >= 2 && (isOpcode(split->stringArray[1]) || isDirective(split->stringArray[1]))) {
                strcpy(symbolTable->symbols[currentSymbol].name, split->stringArray[0]);
                symbolTable->symbols[currentSymbol].lineNumber = lineNumber;
                symbolTable->symbols[currentSymbol].instructions = malloc(sizeof (instruction));
                symbolTable->symbols[currentSymbol].allocatedInstructionAmount = 1;
                symbolTable->symbols[currentSymbol].numInstructions = 1;
                // Saves the instruction can be a opcode or directive
                symbolTable->symbols[currentSymbol].instructions[0].instruction = malloc(strlen(split->stringArray[1]) + 1);
                strcpy(symbolTable->symbols[currentSymbol].instructions[0].instruction, split->stringArray[1]);
                // Storing the args
                for (int i = 2, j = 0; i < split->numStrings; i++, j++) {
                    symbolTable->symbols[currentSymbol].instructions[0].instruction[j] = malloc(strlen(split->stringArray[i] + 1));
                    strcpy(symbolTable->symbols[currentSymbol].instructions[0].instruction[j], split->stringArray[i]);
                    symbolTable->symbols[currentSymbol].instructions[0].numOperands = j;
                }

                // Start directive check
                if (!strcmp(symbolTable->symbols[currentSymbol].instructions[0].instruction, "START") && !seenStart) {
                   address = atoi(symbolTable->symbols[currentSymbol].instructions[0].operands[0]);
                   seenStart = true;
                }

            } else {
                freeSplit(split);
                perror("Error");
                return NULL;
            }
            freeSplit(split);
            
        } else {
            // Add instructions to symbol
        }
        lineNumber++;
    }
}

struct stringArray* stringSplit(char *string, char *delim) {
    struct stringArray *split = malloc(sizeof(struct stringArray));
    split->allocatedAmount = 2;
    split->numStrings = 0;
    split->stringArray = malloc(sizeof(char *) * split->allocatedAmount);
    char *splitTemp = strtok(string, delim);
    while (splitTemp != NULL) {
        if (splitTemp[0] == '#' || splitTemp[0] == '\n') {
            break;
        }
        if (split->numStrings > split->allocatedAmount) {
            split->allocatedAmount = split->allocatedAmount * 2;
            split->stringArray = realloc(split->stringArray, split->allocatedAmount);
        }
        split->stringArray[split->numStrings] = malloc(strlen(splitTemp));
        strcpy(split->stringArray[split->numStrings], splitTemp);
        splitTemp = strtok(NULL, delim);
        split->numStrings++;
    }
    return split;
}

void freeSplit(struct stringArray *split) {
    for (int i = 0; i < split->numStrings; i++) {
        free(split->stringArray[i]);
    }
    free(split->stringArray);
    free(split);
}