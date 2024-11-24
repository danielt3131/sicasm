/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "createObjectFile.h"
#include <string.h>
#include <stdlib.h>
#include "tables.h"
#include "checker.h"
#include "freeObjectFile.h"
#include <ctype.h>
#include "fileBuffer.h"

objectFile* createObjectFile(struct symbolTable *symbolTable, fileBuffer *fileBuf) {
    char *currentLine;
    char buffer[100];
    char tRecordBuffer[61];
    tRecordBuffer[0] = '\0';
    int tRecordAddress = 0;
    objectFile* objFile = malloc(sizeof(objectFile)); 
    struct symbolTable *table = symbolTable;

    // Put in H record
    sprintf(buffer, "H%-6s%06X%06X\n", table->symbols[0].name, table->symbols[0].address, table->symbols[table->numberOfSymbols - 1].address - table->symbols[0].address);
    int length = strlen(buffer);
    objFile->hRecord = malloc(sizeof(char) * (length + 1));
    strcpy(objFile->hRecord, buffer);
    /*
    // Consume start directive
    for (int i = 0; i < table->symbols[0].lineNumber; i++) {
        fgets(currentLine, 1000, assemblyFile);
    }
    */
    //printf(buffer, "H%s %05X%05X\n", table->symbols[0].name, table->symbols[0].address, table->symbols[table->numberOfSymbols - 1].address - table->symbols[0].address);
    buffer[0] = '\0';

    // T & M records
    int address = table->symbols[0].address;
    objFile->tRecords = malloc(sizeof (struct stringArray));
    objFile->tRecords->allocatedAmount = 5;
    objFile->tRecords->numStrings = 0;
    objFile->tRecords->stringArray = malloc(objFile->tRecords->allocatedAmount * sizeof(char *));

    objFile->mRecords = malloc(sizeof (struct stringArray));
    objFile->mRecords->allocatedAmount = 5;
    objFile->mRecords->numStrings = 0;
    objFile->mRecords->stringArray = malloc(objFile->mRecords->allocatedAmount * sizeof(char *));
    objFile->eRecord = malloc(10);
    int symbolIndex = 1;
    int lineNumber = table->symbols[0].lineNumber + 1;
    bool firstExecInstruction = false;
    int firstExecInstructionAddress = 0;
    int objCode = 0;
    for (int i = 1; i < fileBuf->numLines; i++) {
        currentLine = fileBuf->lines[i];
        lineNumber = fileBuf->lineNumbers[i];
        /*
        if (objFile->tRecords->numStrings >= objFile->tRecords->allocatedAmount) {
            objFile->tRecords->allocatedAmount *= 2;
            objFile->tRecords->stringArray = realloc(objFile->tRecords->stringArray, sizeof(char *) * objFile->tRecords->allocatedAmount);
        }
        */
        if (objFile->mRecords->numStrings >= objFile->mRecords->allocatedAmount) {
            objFile->mRecords->allocatedAmount *= 2;
            objFile->mRecords->stringArray = realloc(objFile->mRecords->stringArray, sizeof(char *) * objFile->mRecords->allocatedAmount);
        }
        //removeCR(currentLine); // Convert CRLF to LF
        if(!isspace(currentLine[0])) {
            struct stringArray *split = stringSplit(currentLine, "\t\n");
            address = table->symbols[symbolIndex].address;
            symbolIndex++;
            if (isDirective(split->stringArray[1])) {
                if (!strcmp(split->stringArray[1], "WORD")) {
                    int value = atoi(split->stringArray[2]);
                    sprintf(buffer, "%06X", value);
                    addTRecord(objFile, address, buffer, &tRecordAddress, tRecordBuffer, true);
                } else if (!strcmp(split->stringArray[1], "BYTE") && split->stringArray[2][0] == 'C') {
                    int i = 2;
                    char tmpBuffer[4];
                    int stringLength = strlen(split->stringArray[2]);
                    char *byteBuffer = malloc(2 * stringLength + 2);
                    memset(byteBuffer, 0, (2 * stringLength + 1));
                    while (split->stringArray[2][i] != '\'') {
                        sprintf(tmpBuffer, "%02X", split->stringArray[2][i]);
                        strcat(byteBuffer, tmpBuffer);
                        i++;
                    }
                    int length = strlen(byteBuffer);
                    char *substrPos = byteBuffer;
                    while (length > 60) {
                        char substr[61];
                        strncpy(substr, substrPos, 60);
                        length = length - 60;

                        substrPos += 60;
                        substr[60] = '\0';
                        addTRecord(objFile, address, substr, &tRecordAddress, tRecordBuffer, false);
                        address += 30;
                    }

                    if (strlen(tRecordBuffer) == 0) {
                        addTRecord(objFile, address, substrPos, &tRecordAddress, tRecordBuffer, false);
                    } else {
                        addTRecord(objFile, address, substrPos, &tRecordAddress, tRecordBuffer, true);
                    }

                    //addTRecord(objFile, address, substrPos, &tRecordAddress, tRecordBuffer, false);
                    free(byteBuffer);
                } else if (!strcmp(split->stringArray[1], "BYTE") && split->stringArray[2][0] == 'X') {
                    int i = 2;
                    char tmpBuffer[4];
                    while (split->stringArray[2][i] != '\'') {
                        sprintf(tmpBuffer, "%c", split->stringArray[2][i]);
                        strcat(buffer, tmpBuffer);
                        i++;
                    }
                    addTRecord(objFile, address, buffer, &tRecordAddress, tRecordBuffer, true);
                } else if (!strcmp(split->stringArray[1], "END")) {
                    if (retrieveAddress(table, split->stringArray[2]) == OBJCODE_ERROR) {
                        fprintf(stderr, "Error %s is not a valid instruction for END directive at line: %d\r\n", split->stringArray[2], lineNumber);
                        freeSplit(split);
                        freeSymbolTable(table);
                        freeObjectFile(objFile);
                        return NULL;
                    }
                    freeSplit(split);
                    break;
                } else {
                    if (strlen(tRecordBuffer) != 0) {
                        printTRecord(objFile, tRecordAddress, tRecordBuffer);
                    }
                }
            } else {
                if (!firstExecInstruction) {
                    firstExecInstruction = true;
                    firstExecInstructionAddress = address;
                }

                objCode = objcodeCreate(split->stringArray[1], false, split->stringArray[2], table, lineNumber);
                if (objCode == OBJCODE_ERROR) {
                    freeSplit(split);
                    freeSymbolTable(table);
                    freeObjectFile(objFile);
                    return NULL;
                }
                sprintf(buffer, "%06X", objCode);
                addTRecord(objFile, address, buffer, &tRecordAddress, tRecordBuffer, true);
                addMRecord(objFile, address + 1, table->symbols[0].address, table->symbols[0].name);
            }
            freeSplit(split);

        } else {
            struct stringArray *split = stringSplit(currentLine, "\t\n,");
            if (split->numStrings == 3 && split->stringArray[2][0] == 'X' && strlen(split->stringArray[2]) == 1) {
                objCode = objcodeCreate(split->stringArray[0], true, split->stringArray[1], table, lineNumber);
                if (objCode == OBJCODE_ERROR) {
                    freeSplit(split);
                    freeSymbolTable(table);
                    freeObjectFile(objFile);
                    return NULL;
                }
                sprintf(buffer, "%06X", objCode);
                addMRecord(objFile, address + 4, table->symbols[0].address, table->symbols[0].name);

            } else {
                if (!strcmp("RSUB", split->stringArray[0])) {
                    sprintf(buffer, "4C0000");
                } else {
                    objCode = objcodeCreate(split->stringArray[0], false, split->stringArray[1], table, lineNumber);
                    if (objCode == OBJCODE_ERROR) {
                        freeSplit(split);
                        freeSymbolTable(table);
                        freeObjectFile(objFile);
                        return NULL;
                    }
                    sprintf(buffer, "%06X", objCode);
                    addMRecord(objFile, address + 4, table->symbols[0].address, table->symbols[0].name);
                }

            }
            freeSplit(split);
            address += 3;
            addTRecord(objFile, address, buffer, &tRecordAddress, tRecordBuffer, true);

        }
    }
    sprintf(objFile->eRecord,  "E%06X\n", firstExecInstructionAddress);
    if (strlen(tRecordBuffer) != 0) {
        printTRecord(objFile, tRecordAddress, tRecordBuffer);
    }
    return objFile;
}

/**
 * Removes carriage return CR / 0x1D / 13 from a string
 * @param str The string to remove the carriage return
 */
void removeCR(char *str) {
    int length = strlen(str);
    int i;
    for (i = 0; i < length; i++) {
        if (str[i] == '\r') {
            str[i] = '\n';
        }
    }
    str[i+1] = '\0';
}

void addTRecord(objectFile* objFile, int address, char *objcode, int *startingAddress, char *tRecordBuffer, bool combine) {
    int length = strlen(tRecordBuffer);
    if (combine) {
        if (length == 0) {
            (*startingAddress) = address;
        }
        if (length + strlen(objcode) > 60) {
            printTRecord(objFile, *startingAddress, tRecordBuffer);
            *startingAddress = address;
        }
        strcat(tRecordBuffer, objcode);
        objcode[0] = '\0';
    } else {
        if (length != 0) {
            printTRecord(objFile, *startingAddress, tRecordBuffer);
        }
        printTRecord(objFile, address, objcode);
    }
}
void printTRecord(objectFile *objFile, int address, char *objcode) {
    int objCodeLength = strlen(objcode) / 2;
    char *tmpBuffer = malloc(100);
    sprintf(tmpBuffer, "T%06X%02X%s\r\n", address, objCodeLength, objcode);
    if (objFile->tRecords->numStrings >= objFile->tRecords->allocatedAmount) {
        objFile->tRecords->allocatedAmount *= 2;
        objFile->tRecords->stringArray = realloc(objFile->tRecords->stringArray, objFile->tRecords->allocatedAmount * sizeof(struct stringArray));
    }
    objFile->tRecords->stringArray[objFile->tRecords->numStrings] = malloc((strlen(tmpBuffer) + 1) * sizeof(char));
    //printf("%s", tmpBuffer);
    strcpy(objFile->tRecords->stringArray[objFile->tRecords->numStrings], tmpBuffer);
    free(tmpBuffer);
    objFile->tRecords->numStrings++;
    objcode[0] = '\0';
}
void addMRecord(objectFile* objFile, int address, int startAddress, const char *name) {
    char mBuffer[16];
    if (address > startAddress) {
        sprintf(mBuffer, "M%06X04+%s\n", address, name);
    } else {
        sprintf(mBuffer, "M%06X04-%s\n", address, name);
    }
    objFile->mRecords->stringArray[objFile->mRecords->numStrings] = malloc((strlen(mBuffer) + 1) * sizeof(char));
    strcpy(objFile->mRecords->stringArray[objFile->mRecords->numStrings], mBuffer);
    objFile->mRecords->numStrings++;
   // printf("%s", mBuffer);

}

/*
 * Take in the opcode and flag and return decimal representation of the
 * hex
 * 
 * return: decimal representation of hex of the input combined
 */
int opAndFlagsBit(int opcode, int n, int i, int x, int b, int p, int e) {
    char binary[13];
    opcode >>= 2;


    for(int x = 5; x >= 0; x--) {
        binary[x] = opcode%2 + '0';
        opcode >>= 1;
    }
    binary[6] = n + '0';
    binary[7] = i + '0';
    binary[8] = x + '0';
    binary[9] = b + '0';
    binary[10] = p + '0';
    binary[11] = e + '0';
    binary[12] = '\0';

    return (int)strtol(binary, NULL, 2);
}

/*
 * generate hex object code for BYTE directive to the * max length specified or entire length of string, * * whichever is smaller. Return the string address of * where it stopped
 *
 * str: The string to parse
 * mode: which BYTE mode, either 'C' or 'X"
 * allowHexLen: Maximum length allowed
 * output: result will store in this
 */
char* getJustEnoughByteHex(char* str, char mode, int allowHexLen, char** output) {
  if(allowHexLen%2 != 0) return str; //please make sure allowHexLen is even


  char* result;
  if(mode == 'C') {
    int resultLen = strlen(str)*2 < allowHexLen ? strlen(str)*2 : allowHexLen;
    result = calloc(resultLen + 1, sizeof(char));


    for(int x = 0; x < resultLen; x+=2) {
      sprintf(result+x, "%X", *str);
      str++;
    }
  }
  else if (mode == 'X') {
    int resultLen = strlen(str) < allowHexLen ? strlen(str) : allowHexLen;
    result = calloc(resultLen + 1, sizeof(char));
   
    for(int x = 0; x < resultLen; x++) {
      sprintf(result+x, "%c", *str);
      str++;
    }
  }
  *output = result;
  return str;
}
