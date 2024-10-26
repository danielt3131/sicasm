#include "tables.h"
#include <stdlib.h>
#include <string.h>

struct stringArray* stringSplit(char *string, char *delim) {
    struct stringArray *split = malloc(sizeof(struct stringArray));
    split->allocatedAmount = 4;
    split->numStrings = 0;
    split->stringArray = malloc(sizeof(char *) * split->allocatedAmount);
    char *splitTemp = strtok(string, delim);
    while (splitTemp != NULL) {
        if (splitTemp[0] == '#' || splitTemp[0] == '\n') {
            break;
        }
        if (split->numStrings >= split->allocatedAmount) {
            split->allocatedAmount = split->allocatedAmount * 2;
            split->stringArray = realloc(split->stringArray, split->allocatedAmount);
        }
        split->stringArray[split->numStrings] = malloc(strlen(splitTemp) + 1);
        strcpy(split->stringArray[split->numStrings], splitTemp);
        splitTemp = strtok(NULL, delim);
        split->numStrings++;
    }
    return split;
}

void freeSplit(struct stringArray *split) {
    for (int i = 0; i < split->numStrings; i++) {
        free(split->stringArray[i]);
    }
    free(split->stringArray);
    free(split);
}