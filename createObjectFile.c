#include "createObjectFile.h"
#include <string.h>
#include <stdlib.h>
#include "tables.h"
#include "checker.h"
#include "freeObjectFile.h"
#include <ctype.h>

objectFile* createObjectFile(struct symbolTable *symbolTable, FILE *assemblyFile) {
    char currentLine[1000];
    char buffer[100];
    objectFile* objFile = malloc(sizeof(objectFile)); 
    struct symbolTable *table = symbolTable;

    // Put in H record
    sprintf(buffer, "H%s %05X%05X\n", table->symbols[0].name, table->symbols[0].address, table->symbols[table->numberOfSymbols - 1].address - table->symbols[0].address);
    int length = strlen(buffer);
    objFile->hRecord = malloc(sizeof(char) * (length + 1));
    strcpy(objFile->hRecord, buffer);

    // Consume start directive
    for (int i = 0; i < table->symbols[0].lineNumber; i++) {
        fgets(currentLine, 1000, assemblyFile);
    }
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

    int symbolIndex = 1;
    int lineNumber = table->symbols[0].lineNumber + 1;
    bool firstExecInstruction = false;
    int firstExecInstructionAddress = 0;
    int objCode = 0;
    while (fgets(currentLine, 1000, assemblyFile) != NULL) {
        if (currentLine[0] != '#') {
            if (objFile->tRecords->numStrings >= objFile->tRecords->allocatedAmount) {
                objFile->tRecords->allocatedAmount *= 2;
                objFile->tRecords->stringArray = realloc(objFile->tRecords->stringArray, sizeof(char *) * objFile->tRecords->allocatedAmount);
            }

            if (objFile->mRecords->numStrings >= objFile->mRecords->allocatedAmount) {
                objFile->mRecords->allocatedAmount *= 2;
                objFile->mRecords->stringArray = realloc(objFile->mRecords->stringArray, sizeof(char *) * objFile->mRecords->allocatedAmount);
            }
            removeCR(currentLine); // Convert CRLF to LF
            if(!isspace(currentLine[0])) {
                struct stringArray *split = stringSplit(currentLine, "\t\n");
                address = table->symbols[symbolIndex].address;
                symbolIndex++;
                if (isDirective(split->stringArray[1])) {
                    if (!strcmp(split->stringArray[1], "WORD")) {
                        int value = atoi(split->stringArray[2]);
                        sprintf(buffer, "%06X", value);
                        addTRecord(objFile, address, buffer);
                    } else if (!strcmp(split->stringArray[1], "BYTE") && split->stringArray[2][0] == 'C') {
                        int i = 2;
                        char tmpBuffer[4];
                        int stringLength = strlen(split->stringArray[2]);
                        char *byteBuffer = malloc(2 * stringLength + 1);
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
                            addTRecord(objFile, address, substr);
                            address += 30;
                        }
                        addTRecord(objFile, address, substrPos);
                        free(byteBuffer);
                    } else if (!strcmp(split->stringArray[1], "BYTE") && split->stringArray[2][0] == 'X') {
                        int i = 2;
                        char tmpBuffer[4];
                        while (split->stringArray[2][i] != '\'') {
                            sprintf(tmpBuffer, "%c", split->stringArray[2][i]);
                            strcat(buffer, tmpBuffer);
                            i++;
                        }
                        addTRecord(objFile, address, buffer);
                    } else if (!strcmp(split->stringArray[1], "END")) {
                        freeSplit(split);
                        break;
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
                    addTRecord(objFile, address, buffer);
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
                        sprintf(buffer, "6C0000");
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
                addTRecord(objFile, address, buffer);

            }
        }
        lineNumber++;
    }
    objFile->eRecord = malloc(8);
    sprintf(objFile->eRecord, "E%05X\n", firstExecInstructionAddress);
    fclose(assemblyFile);
    return objFile;
}

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

void addTRecord(objectFile* objFile, int address, const char *objcode) {
    int objCodeLength = strlen(objcode) / 2;
    char *tmpBuffer = malloc(80);
    sprintf(tmpBuffer, "T%06X%02X%s\r\n", address, objCodeLength, objcode);
    objFile->tRecords->stringArray[objFile->tRecords->numStrings] = malloc((strlen(tmpBuffer) + 1) * sizeof(char));
    //printf("%s", tmpBuffer);
    strcpy(objFile->tRecords->stringArray[objFile->tRecords->numStrings], tmpBuffer);
    free(tmpBuffer);
    objFile->tRecords->numStrings++;
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
