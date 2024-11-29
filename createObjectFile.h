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
objectFile* createObjectFile(struct symbolTable *table, fileBuffer *fileBuf);
int getTRecords(struct symbolTable *symbolTable, fileBuffer *fileBuf, recordList* recordTable);
void printTRecord(objectFile *objFile, int address, char *objcode);
void addMRecord(objectFile* objFile, int address, int startAddress, const char *name);
char* getJustEnoughByteHex(char* str, char mode, int allowHexLen, char** output);
int opAndFlagsBit(int opcode, int n, int i, int x, int b, int p, int e);
int getOperandNumber(char* operand);
int getFlagsInfo(char* ins, char* operand, int curAdd, int operAdd, int baseAdd, int* n, int* i, int* x, int* b, int* p, int* e);
char* removeFirstFlagLetter(char* str);
int getObjCodeFormat3N4(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output);
int getTObjCode(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output);
long getWordNum(char* operand);
char* createTRecord(int startAdd, char* Objcodes);
int getObjCodeFormat1(char* ins, char** output);
int getObjCodeFormat2(char* ins, char* operand, char** output);
#endif