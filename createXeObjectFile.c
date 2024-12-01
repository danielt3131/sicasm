/**
 * @author Tianyu Wu
 * @author Samuel Gray
 * @author Daniel J. Thompson (N01568044)
 */
#include "createXeObjectFile.h"
#include <string.h>
#include <stdlib.h>
#include "tables.h"
#include "checker.h"
#include <ctype.h>
#include "fileBuffer.h"

objectFile* createXeObjectFile(struct symbolTable *symbolTable, fileBuffer *fileBuf) {
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
    objectFile* objFile = malloc(sizeof(objectFile));

    // Variables to store program name, starting address, and program length
    char *programName = NULL;
    int startAddress = 0;
    int programLength = 0;

    // Parse the first line to get program name and start address
    if (fileBuf->numLines > 0) {
        char *firstLine = strdup(fileBuf->lines[0]); // Duplicate the first line to avoid modifying the original
        char *token = strtok(firstLine, " \t\n"); // Tokenize by spaces, tabs, and newlines

        // Get program name
        if (token != NULL) {
            programName = strdup(token); // Store the program name
        }

        // Get directive (should be "START")
        token = strtok(NULL, " \t\n");
        if (token != NULL && strcmp(token, "START") == 0) {
            // Get starting address
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                startAddress = (int)strtol(token, NULL, 16); // Convert from hex string to int
            }
        }
        free(firstLine); // Free the duplicate line
    }

    // Calculate the program length
    if (fileBuf->numLines > 0) {
        int lastAddress = fileBuf->address[fileBuf->numLines - 1]; // Get the last address
        programLength = lastAddress - startAddress;
    }
    // Construct the header record and add to objectFile*
    objFile->hRecord = calloc(20, sizeof(char));
    sprintf(objFile->hRecord, "H%-6s%06X%06X\n", programName, startAddress, programLength);

    // Debug output to verify program name, starting address, and program length
    if (programName != NULL) {
        printf("Program Name: %s\n", programName);
    }
    printf("Start Address: %04X\n", startAddress);
    printf("Program Length: %04X bytes\n", programLength);

    // Create the record list for object file
    recordList *tRecord = calloc(1, sizeof(recordList));
    recordList *mRecord = calloc(1, sizeof(recordList));

    // Generate text records (T-records)

    int firstExecInstructionAddress;
    int error = getTAndMRecords(symbolTable, fileBuf, tRecord, mRecord, programName, &firstExecInstructionAddress);
    if (error) {
        fprintf(stderr, "Error generating text records: %d\n", error);
    }
    // Free the program name
    free(programName);

    //printf("%d\n", error);
    objFile->tRecords = printRecordTable(*tRecord);
    objFile->eRecord = malloc(10);
    objFile->mRecords = printRecordTable(*mRecord);
    sprintf(objFile->eRecord,  "E%06X\n", firstExecInstructionAddress);
    freeRecord(tRecord);
    freeRecord(mRecord);
    //return record;
    return objFile;
}
int getTAndMRecords(struct symbolTable *symbolTable, fileBuffer *fileBuf, recordList* tRecordTable, recordList* mRecordTable, char* programName, int *firstExecInstructionAddress) {
    bool firstExecInstruction = false;
    char TObjCode[61]="";
    int startAdd = fileBuf->address[0];
    int baseAdd = 0;
    int needNewRecord = 0; //Flag for going to next T-record
    int errorCode = 0;
    for(int x = 1; x < fileBuf->numLines-1; x++) {
        char* insOrDir;
        char* operand;
        char* newObjCode = NULL;
        char* curLine = fileBuf->lines[x];
        int curAdd = fileBuf->address[x];
        int nextStartAdd = curAdd;
        int requireMRecord = 0;
        printf("%X %s\n", curAdd, fileBuf->lines[x]);

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
            if (!firstExecInstruction) {
                firstExecInstruction = true;
                *firstExecInstructionAddress = fileBuf->address[x]; // The previous address
            }
            errorCode = validateXeInsFormat(symbolTable, insOrDir, operand);
            if(errorCode) {
                errorOutput(x+1, insOrDir, operand, errorCode);
                return errorCode;
            }

            newObjCode = calloc(9, sizeof(char));

            int operAdd = 0;
            if(operand != NULL && getXeFormat(insOrDir) != 2) operAdd = getOperAddress(symbolTable, operand);
            if(operAdd == -1) {
                freeSplit(strArr);
                free(newObjCode);
                return -1; //Error occur
            }
            errorCode = getTObjCode(insOrDir, operand, curAdd, baseAdd, operAdd, &requireMRecord, &newObjCode);

            if(errorCode) {
                freeSplit(strArr);
                free(newObjCode);
                errorOutput(x+1, insOrDir, operand, errorCode);
                return errorCode;
            }

            if(requireMRecord) {
                char* newR = getMObjCode(curAdd, programName);
                insertRecord(mRecordTable, newR);
            }

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
                freeSplit(strArr);
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
                nextStartAdd = fileBuf->address[x+1];
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
                        insertRecord(tRecordTable, newRecord);
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
                insertRecord(tRecordTable, newRecord);
                TObjCode[0] = '\0';
                if(newObjCode != NULL) {
                    strcat(TObjCode, newObjCode);
                    free(newObjCode);
                }

                newObjCode = NULL;
            }
            needNewRecord = 0;
            startAdd = nextStartAdd;

        }
        freeSplit(strArr);
    }
    if(TObjCode[0] != '\0') {
        char* newRecord = createTRecord(startAdd, TObjCode);
        insertRecord(tRecordTable, newRecord);
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

int getTObjCode(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, int* requireMRecord, char** output) {
    int errorCode = 0;
    switch(getXeFormat(ins)) {
        case 1:
            errorCode = getObjCodeFormat1(ins, output);
            break;
        case 2:
            errorCode = getObjCodeFormat2(ins, operand, output);
            break;
        case 3:
            errorCode = getObjCodeFormat3N4(ins, operand, curAdd, baseAdd, operAdd, requireMRecord, output);
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
    freeSplit(split);
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

int getObjCodeFormat3N4(char* ins, char* operand, int curAdd, int baseAdd, int operAdd, int* requireMRecord, char** output) {
    int n = 0, i = 0, x = 0, b = 0, p = 0, e = 0;    
    int errorCode = getFlagsInfo(ins, operand, curAdd, operAdd, baseAdd, &n, &i, &x, &b, &p, &e);
    if(!errorCode) {
        int upper3Hex = opAndFlagsBit(getOpcodeValue(removeFirstFlagLetter(ins)), n, i, x, b, p, e);
        int lowerHex = operAdd;
        int testInt = -1;
        if(i) {
            testInt = getOperandNumber(operand); //Testing if is a immediate integer
            if(testInt != -1) lowerHex = testInt; //If the operand is a immediate integer, change the displacement to that value
        }
        int pcAdd = (e) ? (curAdd+4) : (curAdd+3);
        if(p) lowerHex = operAdd - (pcAdd); //This should not happen for immediate integer, checked by the getFlagsInfo
        if(b) lowerHex = operAdd - baseAdd; //This also shouldn't happen for immediate integer, 
        if(e)
            sprintf(*output, "%03X%05X", upper3Hex, lowerHex & 0xFFFFF);
        else
            sprintf(*output, "%03X%03X", upper3Hex, lowerHex & 0xFFF);
        
        if(e) {
            if(testInt == -1) *requireMRecord = 1;
        }
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

int validateXeInsFormat(struct symbolTable* symbolTable, char* ins, char* operand) {
    int format = getXeFormat(ins);
    if(format == -1) return 10; //Invalid instruction or flag indicator ;

    if(format != 3 && *ins == '+') return 11; //Flag indicator + are only allowed for format 3 and 4

    if(format == 1 || strcmp(removeFirstFlagLetter(ins), "RSUB") == 0)
        return (operand == NULL) ? 0 : 12;  //Operand not allowed for those format

    if(operand == NULL) return 13; //Missing operand

    if(format == 2) {
        struct stringArray* split = stringSplit(operand, ",");
        if(split->numStrings > 2) {
            freeSplit(split);
            return 14; //format 2 allow maximum of 2 register
        }
        int reg1 = 0, reg2 = 0;

        reg1 = getRegisterNum(split->stringArray[0]);

        if(split->numStrings > 1) {
            reg2 = getRegisterNum(split->stringArray[1]);
        }

        freeSplit(split);
        if(reg1 == -1 || reg2 == -1) {
            return 15; //Invalid register
        }
    }

    if(format == 3) {
        struct stringArray* split = stringSplit(operand, "\t\n");
        if(split->numStrings > 1) {
            freeSplit(split);
            return 16; //Multiple operand not allow
        }
        int address = getOperAddress(symbolTable, operand);
        if (address == -1) {
            freeSplit(split);
            return 17; // Operand not in the symbol table
        }
        freeSplit(split);
    }

    return 0;
}

char* getMObjCode(int curAdd, char* programName) {
    char* r = calloc(10+strlen(programName)+1, sizeof(char));

    sprintf(r, "M%06X%02X+%s", curAdd+1, 5, programName);
    return r;
}

void errorOutput(int lineNum, char* insOrDir, char* operand, int errorCode) {
    switch (errorCode) {
        case 10:
            printf("Line: %d - %s is a invalid instruction\n", lineNum, insOrDir);
            break;
        case 11:
            printf("Line: %d - %s instruction does not support format 4\n", lineNum, insOrDir);
            break;
        case 12:
            printf("Line: %d - %s instruction does not allow operand\n", lineNum, insOrDir);
            break;
        case 13:
            printf("Line: %d - %s instruction missing operand(s)\n", lineNum, insOrDir);
            break;
        case 14:
            printf("Line: %d - %s instruction only allow a maximum of 2 input register\n", lineNum, insOrDir);
            break;
        case 15:
            printf("Line: %d - %s instruction contain invalid register name\n", lineNum, insOrDir);
            break;
        case 16:
            printf("Line: %d - %s instruction does not allow multiple operand\n", lineNum, insOrDir);
            break;
        case 17:
            printf("Line: %d - %s does not exist in the symbol table\n", lineNum, operand);
            break;
        case 21:
            printf("Line: %d - %s contain mutiple addressing mode\n", lineNum, operand); //This might never run
            break;
        case 22:
            printf("Line: %d - displacement to %s does not fit within 12 bits, please use format 4 or use a different base address\n", lineNum, operand);

    }
}