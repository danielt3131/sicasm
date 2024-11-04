/**
 * @author Daniel J. Thompson (N01568044)
 */
#ifndef TABLES_H
#define TABLES_H
#include <stdio.h>
#include <stdbool.h>
/*
typedef struct {
    char *instruction;
    short numOperands;
    char *operands[2];
} instruction;
*/
typedef struct {
    char **lines;
    int numLines;
    int allocationAmount;
    int *lineNumbers;
} fileBuffer;

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

typedef struct {
    char *hRecord;
    struct stringArray* tRecords;
    char *eRecord;
    struct stringArray* mRecords;
} objectFile;


struct stringArray* stringSplit(char *string, char *delim);
void freeSymbolTable(struct symbolTable*);
struct symbolTable* createSymbolTable(fileBuffer *fileBuf, int *numSymbols);
//struct symbolTable* createSymbolTable(FILE *file);
void freeSplit(struct stringArray *split);
#endif