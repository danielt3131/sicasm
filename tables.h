/**
 * @author Daniel J. Thompson (N01568044)
 * @author Tianyu Wu
 * SPDX-License-Identifier: GPL-2.0-or-later
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
    int *address;
} fileBuffer;

typedef struct {
    char *name;
    int address;
    int lineNumber;
    //int numInstructions;
    //int allocatedInstructionAmount;
    //instruction *instructions;
    bool isExternal;
} symbol;

struct symbolTable {
    symbol *symbols;
    int allocatedAmount;
    int numberOfSymbols;
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
    struct stringArray* dRecords;
} objectFile;

typedef struct record {
    char* r;
    struct record* next;
} record;
typedef struct recordList {
    record* head;
    record* tail;
} recordList;

struct stringArray* stringSplit(char *string, char *delim);
void freeSymbolTable(struct symbolTable*);
struct symbolTable* createSymbolTable(fileBuffer *fileBuf, int *numSymbols, bool *isXE);
//struct symbolTable* createSymbolTable(FILE *file);
void freeSplit(struct stringArray *split);
void insertRecord(recordList* table, char* r);
struct stringArray* printRecordTable(recordList table);
void freeRecord(recordList* table);
#endif