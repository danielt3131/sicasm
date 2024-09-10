#ifndef CHECKER_H
#define CHECKER_H
#include <stdio.h>
#include <stdbool.h>
#include "tables.h"
bool isValidSymbol(char *currentSymbol, struct symbolTable* table);
bool isDirective(char *directive);
bool isOpcode(char *opcode);
// Supports 1 and 2 operands
bool isValidOperand(char **operand, char *instruction);
const char *directives[];
const char *opcodes[];
#define NUMBER_OF_DIRECTIVES 8
#define NUMBER_OF_OPCODES 59
bool containsValidCharacters(char *string);
#endif