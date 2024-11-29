/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CHECKER_H
#define CHECKER_H
#include <stdio.h>
#include <stdbool.h>
#include "tables.h"
bool isValidSymbol(char *currentSymbol, struct symbolTable* table, int lineNumber);
bool isDirective(char *directive);
bool isOpcode(char *opcode);
int getRegisterNum(char *registerName);
// Supports 1 and 2 operands
//bool isValidOperand(char **operand, char *instruction);
int getOpcodeValue(const char *opcode);
#define NUMBER_OF_DIRECTIVES 9
#define NUMBER_OF_OPCODES 59
#define NUMBER_OF_XEOPCODES 33
#define MAX_WORD_SIZE 1048575
#define RAM_LIMIT 32767
#define REGISTER_A  0
#define REGISTER_X  1
#define REGISTER_L  2
#define REGISTER_B  3
#define REGISTER_S  4
#define REGISTER_T  5
#define REGISTER_F  6
#define REGISTER_PC 8
#define REGISTER_SW 9
#define NUMBER_OF_REGISTERS 9
#define OBJCODE_ERROR -1
bool containsValidCharacters(char *string);
int getXeFormat(const char *opcode);
int getAddress(struct symbolTable* table, const char *symbol);
bool xeChecker(struct stringArray* split);
#endif