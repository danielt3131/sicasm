/**
 * @author Tianyu Wu
 */
#include "createXeObjectFile.h"
#include <string.h>
#include <stdlib.h>
#include "tables.h"
#include "checker.h"
#include <ctype.h>
#include "fileBuffer.h"

recordList* createXeObjectFile(struct symbolTable *symbolTable, fileBuffer *fileBuf) {
    recordList *record = calloc(1, sizeof(recordList));
    int error = getTRecords(symbolTable, fileBuf, record);
    printf("%d\n",error);
    printRecordTable(*record);
    return record;
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
            if(operand != NULL && getXeFormat(insOrDir) != 2) operAdd = getOperAddress(symbolTable, operand);
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
                sprintf(newObjCode, "%06X", (unsigned int) getWordNum(operand));
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
            errorCode = getObjCodeFormat1(ins, output);
            break;
        case 2:
            errorCode = getObjCodeFormat2(ins, operand, output);
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

int getObjCodeFormat1(char* ins, char** output) {
    int opcode = getOpcodeValue(ins);
    if(opcode == -1) return -1;

    sprintf(*output, "%02X", opcode);
    return 0;
}

int getObjCodeFormat2(char* ins, char* operand, char** output) {
    int opcode = getOpcodeValue(ins);
    if(opcode == -1) return -1;
    int reg1 = 0, reg2 = 0;
    struct stringArray* split = stringSplit(operand, ",");
    reg1 = getRegisterNum(split->stringArray[0]);
    if(split->numStrings >= 2) {
        reg2 = getRegisterNum(split->stringArray[1]);
    }

    if(reg1 == -1 || reg2 == -1) return -1;

    sprintf(*output, "%02X%01X%01X", opcode, reg1, reg2);
    return 0;
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
char* getJustEnoughByteHex(char* str, char mode, unsigned int allowHexLen, char** output) {
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

