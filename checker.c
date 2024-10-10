/**
 * @author Daniel J. Thompson (N01568044)
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
const char *directives[] = {"START","END","BYTE","WORD","RESB","RESW","RESR","EXPORTS"};
const char *opcodes[] = {"ADD","ADDF","ADDR","AND","CLEAR","COMP","COMPF","COMPR","DIV","DIVF","DIVR","FIX","FLOAT",
"HIO","J","JEQ","JGT","JLT","JSUB","LDA","LDB","LDCH","LDF","LDL","LDS","LDT","LDX","LPS","MUL","MULF","MULR","NORM","OR",
"RD","RMO","RSUB","SHIFTL","SHIFTR","SIO","SSK","STA","STB","STCH","STF","STI","STL","STS","STSW","STT","STX","SUB","SUBF","SUBR","SVC","TD","TIO","TIX","TIXR","WD"};
bool isDirective(char *directive) {
    for (int i = 0; i < NUMBER_OF_DIRECTIVES; i++) {
        if(!strcmp(directive, directives[i])) {
            return true;
        }
    }
    return false;
}

bool isOpcode(char *opcode){
    // check aganist list of opcodes
    for (int i = 0; i < NUMBER_OF_DIRECTIVES; i++) {
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