#ifndef TABLES_H
#define TABLES_H
#include <stdio.h>
/*
typedef struct {
    char *instruction;
    short numOperands;
    char *operands[2];
} instruction;
*/

typedef struct {
    char *name;
    int address;
    int lineNumber;
    //int numInstructions;
    //int allocatedInstructionAmount;
    //instruction *instructions;
} symbol;

struct symbolTable {
    symbol *symbols;
    int numberOfSymbols;
    int allocatedAmount;
};

struct stringArray {
    char **stringArray;
    int numStrings;
    int allocatedAmount;
};

struct stringArray* stringSplit(char *string, char *delim);
void freeSymbolTable(struct symbolTable*);
struct symbolTable* createSymbolTable(FILE *file);
#endif