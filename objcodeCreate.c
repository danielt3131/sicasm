/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "createObjectFile.h"
#include <string.h>
#include "checker.h"

int objcodeCreate(const char *opcode, bool addressMode, const char *symbolName, struct symbolTable* table, int lineNumber) {
    char buffer1[20];
    int opcodeValue = getOpcodeValue(opcode);
    //printf("%s\n", symbolName);
    //printf("%02x\n", opcodeValue);
    if (opcodeValue == OBJCODE_ERROR) {
        fprintf(stderr, "%s is not a valid opcode at line %d\r\n", opcode, lineNumber);
        return OBJCODE_ERROR;
    }
    sprintf(buffer1, "%02x", opcodeValue);
    char buffer[10];
    //printf("%s\n", buffer1);
    //printf("%sH", symbolName);
    int address = retrieveAddress(table, symbolName);
    if (address == OBJCODE_ERROR) {
        fprintf(stderr, "Symbol %s not defined at line %d\r\n", symbolName, lineNumber);
        return OBJCODE_ERROR;
    }
    sprintf(buffer, "%04x", address);
    // AddressMode
    //printf("%s | %s\n"
    //printf("%s\n", symbolName);
    strcat(buffer1, buffer);
    //printf("%s\n", buffer1);
    int objcode = 0;
    sscanf(buffer1, "%x", &objcode);
    if (addressMode) {
        objcode += 32768;
    }
    return objcode;
}

int retrieveAddress(struct symbolTable* table, const char *symbolName) {
    for (int i = 0; i < table->numberOfSymbols; i++) {
      //  printf("%s ", table->symbols[i].name);
        if (strcmp(symbolName, table->symbols[i].name) == 0) {
            //printf("%04x\n", table->symbols[i].address);
            return table->symbols[i].address;
        }
    }
    return OBJCODE_ERROR;
}

