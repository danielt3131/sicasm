/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "checker.h"
#include "tables.h"
#include <string.h>
#include <ctype.h>

bool isValidSymbol(char *currentSymbol, struct symbolTable* table, int lineNumber) {
    //printf("%s\n", currentSymbol);
    if (!isalpha(currentSymbol[0])) {
        fprintf(stderr, "Line %d symbol %s isn't valid due to starting with a character that isn't A-Z\r\n", lineNumber, currentSymbol);
        return false;
    }
    if (isDirective(currentSymbol)) {
        fprintf(stderr, "Line %d symbol %s isn't valid due to being a directive\r\n", lineNumber, currentSymbol);
        return false;
    }
    if (isOpcode(currentSymbol)) {
        fprintf(stderr, "Line %d symbol %s isn't valid due to being an opcode A-Z\r\n", lineNumber, currentSymbol);
        return false;
    }
    if (strlen(currentSymbol) > 6) {
        fprintf(stderr, "Line %d symbol %s isn't valid due to being longer than 6 characters\r\n", lineNumber, currentSymbol);
        return false;
    }
    if (!containsValidCharacters(currentSymbol)) {
        fprintf(stderr, "Line %d symbol %s isn't valid because it contains an invalid character\r\n", lineNumber, currentSymbol);
        return false;
    }
    if (table->numberOfSymbols == 0) {
        return true;
    }
    //printf("%d | %s\n", table->numberOfSymbols, currentSymbol);
    for (int i = 0; i < table->numberOfSymbols; i++) {
        if(!strcmp(currentSymbol, table->symbols[i].name)) {
            fprintf(stderr, "Line %d symbol %s isn't valid because the symbol was already declared at Line %d\r\n", lineNumber, currentSymbol, table->symbols[i].lineNumber);
            return false;
        }
    }
    return true;
}


bool containsValidCharacters(char *string) {
    int length = strlen(string);
    for (int i = 0; i < length; i++) {
        if (string[i] == '$') {
            return false;
        }
        if (string[i] == ' ') {
            return false;
        }
        if (string[i] == '!') {
            return false;
        }
        if (string[i] == '=') {
            return false;
        }
        if (string[i] == '+') {
            return false;
        }
        if (string[i] == '-') {
            return false;
        }
        if (string[i] == ',') {
            return false;
        }
        if (string[i] == '@') {
            return false;
        }
    }
    return true; 
}
const char *directives[] = {"START","END","BYTE","WORD","RESB","RESW","RESR","EXPORTS","BASE"};
/**
 * @brief Opcodes names & values in hexadecimal
  */
const char *opcodes[] = {"ADD","ADDF","ADDR","AND","CLEAR","COMP","COMPF","COMPR","DIV","DIVF","DIVR","FIX","FLOAT",
"HIO","J","JEQ","JGT","JLT","JSUB","LDA","LDB","LDCH","LDF","LDL","LDS","LDT","LDX","LPS","MUL","MULF","MULR","NORM","OR",
"RD","RMO","RSUB","SHIFTL","SHIFTR","SIO","SSK","STA","STB","STCH","STF","STI","STL","STS","STSW","STT","STX","SUB","SUBF","SUBR","SVC","TD","TIO","TIX","TIXR","WD"};
const int opcodesValue[] = {0x18, 0x58, 0x90, 0x40, 0xB4, 0x28, 0x88, 0xA0, 0x24, 0x64, 0x9C, 0xC4, 0xC0, 0xF4, 0x3C, 0x30, 0x34, 0x38, 0x48, 0x00, 0x68, 0x50, 
0x70, 0x08, 0x6C, 0x74, 0x04, 0xD0, 0x20, 0x60, 0x98, 0xC8, 0x44, 0xD8, 0xAC, 0x4C, 0xA4, 0xA8, 0xF0, 0xEC, 0x0C, 0x78, 0x54, 0x80, 0xD4, 0x14, 0x7C, 0xE8, 0x84, 0x10, 
0x1C, 0x5C, 0x94, 0xB0, 0xE0, 0xF8, 0x2C, 0xB8, 0xDC};
const int opcodeFormats[] = {3, 3, 2, 3, 2, 3, 3, 2, 3, 3, 2, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 3, 3, 2, 3, 2, 2, 1, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 1, 3, 2, 3};
bool isDirective(char *directive) {
    for (int i = 0; i < NUMBER_OF_DIRECTIVES; i++) {
        if(!strcmp(directive, directives[i])) {
            return true;
        }
    }
    return false;
}

bool isOpcode(char *opcode){
    // check if it can have +
    if(*opcode == '+' && getXeFormat(opcode+1) == 3)
        opcode++;
    else
        return false;
    // check against list of opcodes
    for (int i = 0; i < NUMBER_OF_OPCODES; i++) {
        if(!strcmp(opcode, opcodes[i])) {
            return true;
        }
    }
    return false;
}
/*
// Supports 1 and 2 operands
bool isValidOperand(char **operand, char *instruction){
    for (int i = 0; i < NUMBER_OF_OPCODES; i++) {
        if (!strcmp(opcodes[i], instruction)) {
            return true;
        }
    }
    return false;
}
*/

int getOpcodeValue(const char *opcode) {
    for (int i = 0; i < NUMBER_OF_OPCODES; i++) {
        if (!strcmp(opcode, opcodes[i])) {
            //printf("%x", opcodesValue[i]);
            return opcodesValue[i];
        }
    }
    return OBJCODE_ERROR;
}

int getXeFormat(const char *opcode) {
    if(*opcode == '+') opcode++;
    for (int i = 0; i < NUMBER_OF_OPCODES; i++) {
        if (!strcmp(opcode, opcodes[i])) {
            return opcodeFormats[i];
        }
    }

    return -1;
}

int getAddress(struct symbolTable* table, const char *symbol) {
    for (int i = 0; i < table->numberOfSymbols; i++) {
        if(!strcmp(symbol, table->symbols[i].name)) {
            return table->symbols[i].address;
        }
    }
    return -1;
}
const char *registerNames[] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};
const int registerValues[] = {REGISTER_A, REGISTER_X, REGISTER_L, REGISTER_B, REGISTER_S, REGISTER_T, REGISTER_F, REGISTER_PC, REGISTER_SW};
int getRegisterNum(char *registerName) {
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (!strcmp(registerNames[i], registerName)) {
            return registerValues[i];
        }
    }
    return -1;
}