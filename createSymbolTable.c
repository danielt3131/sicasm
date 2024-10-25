/**
 * @author Daniel J. Thompson (N01568044)
 */
#include "checker.h"
#include "tables.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void freeSplit(struct stringArray *split);

/**
 * @brief Creates a symbol table for the SIC archtecture
 * @param file The file pointer for the assembly file 
 * @return The pointer to the symbol table or NULL if there was an error
 */
struct symbolTable* createSymbolTable(FILE *file) {
    char currentLine[1000];
    struct symbolTable *symbolTable = malloc(sizeof(struct symbolTable));
    symbolTable->symbols = malloc(5 * sizeof(symbol));
    symbolTable->allocatedAmount = 5;
    symbolTable->numberOfSymbols = 0;
    int address = 0;
    int currentSymbol = 0;
    int lineNumber = 1;
    bool seenStart = false;
    bool seenEnd = false;
    while (fgets(currentLine, 1000, file) != NULL) {
        if ((currentLine[0] == '\r' || currentLine[0] == '\n') && !seenEnd) {
            freeSymbolTable(symbolTable);
            fprintf(stderr, "Whitespace found at Line %d\n", lineNumber);
            return NULL;
        }
        if (currentLine[0] != '#') {
            if (!isspace(currentLine[0])) {
                if (currentSymbol >= symbolTable->allocatedAmount) {
                    symbolTable->allocatedAmount = symbolTable->allocatedAmount * 2;
                    symbolTable->symbols = realloc(symbolTable->symbols, sizeof(symbol) * symbolTable->allocatedAmount);
                }
                struct stringArray *split = stringSplit(currentLine, "\t\n");
                if (isValidSymbol(split->stringArray[0], symbolTable, lineNumber) && split->numStrings >= 2) {
                    symbolTable->symbols[currentSymbol].name = malloc(strlen(split->stringArray[0]) + 1);
                    strcpy(symbolTable->symbols[currentSymbol].name, split->stringArray[0]);
                    symbolTable->symbols[currentSymbol].lineNumber = lineNumber;
                    if (!strcmp(split->stringArray[1], "START") && !seenStart) {
                        seenStart = true;
                        sscanf(split->stringArray[2], "%x", &address);
                        //printf("%d %x", address, address);
                        symbolTable->symbols[currentSymbol].address = address;
                    } else if (!strcmp(split->stringArray[1], "RESB")) {
                        symbolTable->symbols[currentSymbol].address = address;
                        address = address + atoi(split->stringArray[2]);
                    } else if (!strcmp(split->stringArray[1], "BYTE")) {
                        if (split->stringArray[2][0] == 'C') {
                            int i = 2;
                            int size = 0;
                            while (split->stringArray[2][i] != '\'' && split->stringArray[2][i] != '\0') {
                                //  printf("%c\n", split->stringArray[2][i]);
                                i++;
                                size++;
                            }
                            // printf("%d\n", size);
                            symbolTable->symbols[currentSymbol].address = address;
                            address += size;
                        } else if (split->stringArray[2][0] == 'X') {
                            int i = 2;
                            int size = 0;
                            while (split->stringArray[2][i] != '\'' && split->stringArray[2][i] != '\0') {
                                if (!(isdigit(split->stringArray[2][i]) || (split->stringArray[2][i] >='A' &&  split->stringArray[2][i] <= 'F'))) {
                                    freeSplit(split);
                                    freeSymbolTable(symbolTable);
                                    fprintf(stderr, "Line %d contains invalid hexadecimal\r\n", lineNumber);
                                    return NULL;
                                }
                                i++;
                                size++;
                            }
                            symbolTable->symbols[currentSymbol].address = address;
                            address += size / 2;
                        } else {
                            freeSplit(split);
                            freeSymbolTable(symbolTable);
                            fprintf(stderr, "Line %d contains invalid BYTE constant\r\n", lineNumber);
                            return NULL;
                        }
                    } else if (!strcmp(split->stringArray[1], "RESW")) {
                        symbolTable->symbols[currentSymbol].address = address;
                        address = address + (atoi(split->stringArray[2]) * 3);
                    } else if (!strcmp(split->stringArray[1], "WORD")) {
                        if (abs(atoi(split->stringArray[2])) <= MAX_WORD_SIZE) {
                            symbolTable->symbols[currentSymbol].address = address;
                            address += 3;
                        } else {
                            fprintf(stderr, "Line %d word constant exceeds 24 bits\r\n", lineNumber);
                            freeSplit(split);
                            freeSymbolTable(symbolTable);
                            return NULL;
                        }
                    } else if (!strcmp(split->stringArray[1], "END")) {
                        seenEnd = true;
                        symbolTable->symbols[currentSymbol].address = address;
                    } else {
                        symbolTable->symbols[currentSymbol].address = address;
                        address += 3;
                    }
                    currentSymbol++;
                    symbolTable->numberOfSymbols++;
                    if (!seenStart) {
                        freeSymbolTable(symbolTable);
                        freeSplit(split);
                        fprintf(stderr, "Start directive not found\r\n");
                        return NULL;
                    }
                } else {
                    //fprintf(stderr, "Line %d symbol %s isn't valid\r\n", lineNumber, split->stringArray[0]);
                    freeSplit(split);
                    freeSymbolTable(symbolTable);
                    return NULL;
                }
                freeSplit(split);
                //puts("Created Symbol\n");
            } else {
                address += 3;
            }
            if (address > RAM_LIMIT) {
                freeSymbolTable(symbolTable);
                fprintf(stderr, "SIC memory exhausted tried to use %x at line %d when the limit is 7FFF\n", address, lineNumber);
                return NULL;
            }
        }
        //printf("%s \t %X\n", currentLine, address);
        lineNumber++;
    }
    symbolTable->allocatedAmount = symbolTable->numberOfSymbols;
    symbolTable->symbols = realloc(symbolTable->symbols, sizeof(symbol) * symbolTable->allocatedAmount);
    return symbolTable;
}


