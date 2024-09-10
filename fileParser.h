#ifndef FILE_PARSER_H
#define FILE_PARSER_H
#include <stdio.h>
#include <stdbool.h>
struct lines* readFile(FILE *file, char *fileName);
struct stringArray {
    char **stringArray;
    int numStrings;
    int allocatedAmount;
    int *type;
};

#define DEFAULT_AMOUNT 2
#define SYMBOL 1
#define DIRECTIVE 2
#define OPCODE 3
#define OPERAND 4
/**
 * numLines is the true number of lines including comments
 * numTokenLines is the number of lines that have been tokenized
 */
struct lines {
    int allocatedAmount;
    int numTokenLines;
    int numLines;
    struct stringArray *tokens;
};
#endif