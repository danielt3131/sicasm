#include "checker.h"
#include "tables.h"
#include <string.h>
#include <ctype.h>

bool isValidSymbol(char *currentSymbol, struct symbolTable* table) {
    //printf("%s\n", currentSymbol);
    if (!isalpha(currentSymbol[0])) {
        return false;
    }
    if (isDirective(currentSymbol)) {
        return false;
    }
    if (isOpcode(currentSymbol)) {
        return false;
    }
    if (strlen(currentSymbol) > 6) {
        return false;
    }
    if (!containsValidCharacters(currentSymbol)) {
        return false;
    }
    if (table->numberOfSymbols == 0) {
        return true;
    }
    for (int i = 0; i < table->numberOfSymbols; i++) {
        if(!strcmp(currentSymbol, table->symbols[i].name)) {
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

// Supports 1 and 2 operands
bool isValidOperand(char **operand, char *instruction){
    for (int i = 0; i < NUMBER_OF_OPCODES; i++) {
        if (!strcmp(opcodes[i], instruction)) {
            return true;
        }
    }
    return false;
}