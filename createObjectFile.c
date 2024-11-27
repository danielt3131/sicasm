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
    // for(int cur = 0; cur < fileBuf->numLines; cur++) {
    //     struct stringArray* lineTest = stringSplit(fileBuf->lines[cur], "\t\n");
    //     char* ins = NULL;
    //     char* operand = NULL;

    //     if(lineTest->numStrings == 1) {
    //         ins = lineTest->stringArray[0];
    //     }
    //     else if(lineTest->numStrings == 2) {
    //         ins = lineTest->stringArray[0];
    //         operand = lineTest->stringArray[1];
    //     }
    //     else if(lineTest->numStrings==3) {
    //         ins = lineTest->stringArray[1];
    //         operand = lineTest->stringArray[2];
    //     }
    //     else {
    //         printf("Got more than 3 string\n");
    //     }
    //     int operAdd = 0;
    //     char* output = calloc(10, sizeof(char));
    //     int errorCode = -1;
    //     if(operand != NULL) {
    //         operAdd= getAddress(symbolTable, stringSplit(removeFirstFlagLetter(operand), ",")->stringArray[0]);
    //     }
    //     if(getXeFormat(removeFirstFlagLetter(ins)) == 3) {
    //         errorCode = getObjCodeFormat3N4(ins, operand, fileBuf->address[cur], 0, operAdd, &output);
    //         if(!errorCode) {
    //             printf("%s\t%s\t%s\t\n", ins, operand, output);
    //             printf("pcAdd: %03X operandAdd: %05X\n", fileBuf->address[cur]+3, operAdd);
    //         }
    //         else
    //             printf("%s\t%s\tError:%d\n", ins, operand, errorCode);
    //     }
    //     else
    //         printf("%s\t%s\tNot a format 3\n", ins, operand);
    // }

    recordList* record = calloc(1, sizeof(recordList));
    int error = getTRecords(symbolTable, fileBuf, record);
    printRecordTable(*record);
    return NULL;

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

int getTRecords(struct symbolTable *symbolTable, fileBuffer *fileBuf, recordList* recordTable) {
    char TObjCode[61]="";
    int startAdd = fileBuf->address[0];
    int baseAdd = 0;
    int needNewRecord = 0; //Flag for going to next T-record
    for(int x = 1; x < fileBuf->numLines-1; x++) {
        char* insOrDir;
        char* operand;
        char* newObjCode;
        char* curLine = fileBuf->lines[x];
        int curAdd = fileBuf->address[x];
        int nextStartAdd = fileBuf->address[x-1];
        struct stringArray* strArr = stringSplit(curLine, "\t\n");
        
        if(isspace(curLine[0])) {
            insOrDir = strArr->stringArray[0];
            operand = strArr->numStrings > 1 ? strArr->stringArray[1] : NULL;
        }
        else {
            insOrDir = strArr->stringArray[1];
            operand = strArr->numStrings > 2 ? strArr->stringArray[2] : NULL;
        }

        //Instruction
        if(!isDirective(insOrDir)) {
            newObjCode = calloc(9, sizeof(char));

            int operAdd = 0;
            if(operand != NULL) operAdd = getOperAddress(symbolTable, operand);
            if(operAdd == -1) return -1; //Error occur

            int errorCode = getTObjCode(insOrDir, operand, curAdd, baseAdd, operAdd, &newObjCode);

            if(errorCode) return errorCode;

            if(strlen(TObjCode) + strlen(newObjCode) <= 60) {
                strcat(TObjCode, newObjCode);
                free(newObjCode);
                newObjCode = NULL;
            }
            else {
                needNewRecord = 1;
            }
        }
        //Directive
        else {
            if(strcmp(insOrDir, "BASE") == 0) {
                baseAdd = getOperAddress(symbolTable, operand);
                continue;
            }
            else if(strcmp(insOrDir, "WORD") == 0) {
                newObjCode = calloc(7, sizeof(char));
                sprintf(newObjCode, "%06X", getWordNum(operand));
                if(strlen(TObjCode) + strlen(newObjCode) <= 60) {
                    strcat(TObjCode, newObjCode);
                    free(newObjCode);
                    newObjCode = NULL;
                }
                else
                    needNewRecord = 1;
            }
            else if(strcmp(insOrDir, "RESW") == 0 || strcmp(insOrDir, "RESB") == 0) {
                needNewRecord =1;
                nextStartAdd = fileBuf->address[x];
            }
            else if(strcmp(insOrDir, "BYTE") == 0) {
                char mode = *operand;
                operand[strlen(operand)-1] = '\0'; //remove the '
                operand+=2;    //Remove mode and '
                char* temp = NULL;
                int byteAdded = 0;
                do {
                    operand = getJustEnoughByteHex(operand, mode, 60-strlen(TObjCode), &temp);
                    strcat(TObjCode, temp);
                    byteAdded = strlen(temp) /2;
                    free(temp);
                    
                    if(strlen(TObjCode) >= 60) {
                        char* newRecord = createTRecord(startAdd, TObjCode);
                        insertRecord(recordTable, newRecord);
                        nextStartAdd+=byteAdded;
                        startAdd = nextStartAdd;
                        TObjCode[0] = '\0';
                    }
                }
                while(*operand != '\0');
            } 

        }

        if(needNewRecord) {
            if(TObjCode[0] != '\0'){
                char* newRecord = createTRecord(startAdd, TObjCode);
                insertRecord(recordTable, newRecord);
                TObjCode[0] = '\0';
                if(newObjCode != NULL) strcat(TObjCode, newObjCode);
                free(newObjCode);
                newObjCode = NULL;
            }
            needNewRecord = 0;
            startAdd = nextStartAdd;

        }

    }
    if(TObjCode[0] != '\0') {
        char* newRecord = createTRecord(startAdd, TObjCode);
        insertRecord(recordTable, newRecord);
    }


    return 0;
}

char* createTRecord(int startAdd, char* Objcodes) {
    char* result = calloc(9 + strlen(Objcodes) + 1, sizeof(char));
    sprintf(result, "T%06X%02X%s", startAdd, strlen(Objcodes)/2, Objcodes);

    return result;
}

long getWordNum(char* operand) {
    char* end;
    long num = strtol(operand, &end, 10);
    if(*end != '\0' || num > 8388607) return -1; //Not a number;
    return num;
}

int getTObjCode(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output) {
    int errorCode = 0;
    switch(getXeFormat(ins)) {
        case 1:
            break;
        case 2:
            break;
        case 3:
            errorCode = getObjCodeFormat3N4(ins, operand, curAdd, baseAdd, operAdd, output);
            break;
        default:
            printf("Error occur\n");
            break;
    }
    return errorCode; // If errorCode is 0, it means no error;
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

/*
 * Will output the flag bit info and check if the flag are logically valid
 * Return 0 if no error occur, else return the error code
 *        20: the instruction does not have format 3
 *        21: Multiple addressing mode not allowed
 *        22: displacement for format 3 does not fit with 12 bits, 
 *                              maybe ask the user to use format 4
 */
int getFlagsInfo(char* ins, char* operand, int curAdd, int operAdd, int baseAdd, int* n, int* i, int* x, int* b, int* p, int* e) {
    if(getXeFormat(removeFirstFlagLetter(ins)) != 3) return 20; //Please only call this when you know format is 3 (or 4)
    
    //Default bit
    *n = 1, *i = 1, *x = 0, *b = 0, *p = 0, *e = 0;

    if(*ins == '+') *e = 1; 
    if(operand) {
        if(*operand == '@') *i = 0;
        if(*operand == '#') *n = 0;
        if(operand[strlen(operand)-1] == 'X') *x = 1;
    }

    //operand can only have one address mode
    if(*x == 1 && *n != 1 && *i != 1) return 21;
    if(*i == 0 && *n != 1 && *x != 0) return 21;
    if(*n == 0 && *i != 1 && *x != 0) return 21;

    if(*e == 1) return 0; //When is format 4, no b or p is needed
    if(!operand) return 0; //When no operand, no b or p is needed
    if(getOperandNumber(operand) != -1) return 0; //If is a immediate integer, no b or p is needed

    int pcAdd = curAdd + 3; //At this point, it can only be format 3, so pc register is 3 byte next.
    if((operAdd - pcAdd >= -2048) && (operAdd - pcAdd <= 2047)) {
        *p = 1;
        return 0;
    }
    else if((operAdd - baseAdd >= 0) && (operAdd - baseAdd <= 4095)) {
        *b = 1;
        return 0;
    }
    else
        return 22; //The displacement does not fit into 12 bits. I think we can just output error and tell user to use format 4
}

/*
 * Get the int representation of the 
 * immediate value. Return -1 if error 
 * occur or is not a immediate integer
 */
int getOperandNumber(char* operand) {
    if(operand == NULL || *operand == '\0') return -1;

    if(*operand != '#') return -1; //Must have immediate indictor in order to allow number
    operand++;
    char *end;

    long num = strtol(operand, &end, 10);
    if(*end != '\0') return -1; //Not a number;
    return num;
}

/*
 * Use it for now, combine this to the check later
 * This could probably be done inside the isOpcode function
 * Just leave it for now, combine into isOpcode later. 
 */
char* removeFirstFlagLetter(char* str) {
    switch (*str) {
        case '+':
        case '@':
        case '#':
        str++;
    }
    return str;
}

int getObjCodeFormat3N4(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, char** output) {
    int n = 0, i = 0, x = 0, b = 0, p = 0, e = 0;    
    int errorCode = getFlagsInfo(ins, operand, curAdd, operAdd, baseAdd, &n, &i, &x, &b, &p, &e);
    int length = (e == 1) ? 4 : 3;
    if(!errorCode) {
        int upper3Hex = opAndFlagsBit(getOpcodeValue(removeFirstFlagLetter(ins)), n, i, x, b, p, e);
        int lowerHex = operAdd;

        if(i) {
            int testInt = getOperandNumber(operand); //Testing if is a immediate integer
            if(testInt != -1) lowerHex = testInt; //If the operand is a immediate integer, change the displacement to that value
        }
        if(p) lowerHex = operAdd - (curAdd); //This should not happen for immediate integer, checked by the getFlagsInfo
        if(b) lowerHex = operAdd - baseAdd; //This also shouldn't happen for immediate integer, 
        if(e)
            sprintf(*output, "%03X%05X", upper3Hex, lowerHex & 0xFFFFF);
        else
            sprintf(*output, "%03X%03X", upper3Hex, lowerHex & 0xFFF);
        
        return 0; //Success
    }
    return errorCode;
}

int getOperAddress(struct symbolTable *symbolTable, char* operand) {
    int operandLen = strlen(operand);

    if(*operand == '#') {
        int testNum = getOperandNumber(operand); 
        if(testNum != -1)
            return testNum;
        operand++; //Meaning is a immedate address. skip the #
    }
    else if(*operand == '@')
        operand++;
    //Testing index addressing mode
    else if(operandLen > 1 && operand[operandLen-1] == 'X' && operand[operandLen-2] == ',') {
        char temp[operandLen -1];
        strncpy(temp, operand, operandLen-2);
        temp[operandLen-2] = '\0';

        return getAddress(symbolTable, temp);
    }

    return getAddress(symbolTable, operand);
}