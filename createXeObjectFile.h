/**
 * @author Tianyu Wu
 */
#ifndef XEOBJCODECREATE_H
#define XEOBJCODECREATE_H
#include <stdio.h>
#include <stdbool.h>
#include "tables.h"
recordList* createXeObjectFile(struct symbolTable *symbolTable, fileBuffer *fileBuf);
int getTRecords(struct symbolTable *symbolTable, fileBuffer *fileBuf, recordList* recordTable);
char* getJustEnoughByteHex(char* str, char mode, unsigned int allowHexLen, char** output);
int opAndFlagsBit(int opcode, int n, int i, int x, int b, int p, int e);
int getOperandNumber(char* operand);
int getFlagsInfo(char* ins, char* operand, int curAdd, int operAdd, int baseAdd, int* n, int* i, int* x, int* b, int* p, int* e);
char* removeFirstFlagLetter(char* str);
int getObjCodeFormat3N4(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output);
int getTObjCode(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output);
long getWordNum(char* operand);
int getOperAddress(struct symbolTable *symbolTable, char* operand);
char* createTRecord(int startAdd, char* Objcodes);
int getObjCodeFormat1(char* ins, char** output);
int getObjCodeFormat2(char* ins, char* operand, char** output);
#endif