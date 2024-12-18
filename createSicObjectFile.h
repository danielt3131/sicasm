/**
 * @author Daniel J. Thompson (N01568044)
 */
#ifndef OBJCODECREATE_H
#define OBJCODECREATE_H
#include <stdio.h>
#include <stdbool.h>
#include "tables.h"
int objcodeCreate(const char *opcode, bool addressMode, const char *symbolName, struct symbolTable* table, int lineNumber);
int retrieveAddress(struct symbolTable* table, const char *symbolName);
void removeCR(char *str);
objectFile* createSicObjectFile(struct symbolTable *table, fileBuffer *fileBuf);
void addTRecord(objectFile* objFile, int address, char *objcode, int *startingAddress, char *tRecordBuffer, bool combine);
void printTRecord(objectFile *objFile, int address, char *objcode);
void addMRecord(objectFile* objFile, int address, int startAddress, const char *name);
#endif