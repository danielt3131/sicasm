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
// Supports 1 and 2 operands
//bool isValidOperand(char **operand, char *instruction);
int getOpcodeValue(const char *opcode);
#define NUMBER_OF_DIRECTIVES 9
#define NUMBER_OF_OPCODES 59
#define MAX_WORD_SIZE 1048575
#define RAM_LIMIT 32767
#define OBJCODE_ERROR -1
bool containsValidCharacters(char *string);
int getXeFormat(const char *opcode);
int getAddress(struct symbolTable* table, const char *symbol);
#endif