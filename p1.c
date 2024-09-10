#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileParser.h"



int main(int argc, char **argv) {
    /*
    if (argc != 2) {
        printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
        return (EXIT_FAILURE);
    }
    */
    FILE *fp = fopen("copymystring.sic", "r");
    //FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("The file %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }

    /*
    char currentLine[100];
    
    struct lines allLines;
    allLines.tokens = (struct stringArray*) malloc(5 * sizeof(struct stringArray));
    allLines.allocatedAmount = 5;
    while (fgets(currentLine, 100, fp) != NULL) {
        if (strlen(currentLine) == 0) {
            return EXIT_FAILURE;
        }
        
    }

    fclose(fp);
    */
    struct lines *allLines = readFile(fp, argv[1]);
    if (allLines == NULL) {
        perror("Error");
        return EXIT_FAILURE;
    }
    //printf("Num Lines: %d\n", allLines->numLines);
    for (int i = 0; i < allLines->numLines; i++) {
        //printf("Num strings: %d\n", allLines->tokens[i].numStrings);
        for (int j = 0; j < allLines->tokens[i].numStrings; j++) {
            //printf("%s %d|%d ", allLines->tokens[i].stringArray[j], i, j);
            printf("%s\n", allLines->tokens[i].stringArray[j]);
            free(allLines->tokens[i].stringArray[j]);
        }
        free(allLines->tokens[i].stringArray);
        //printf("\n");
    }
    free(allLines->tokens);
    free(allLines);
    return (EXIT_SUCCESS);
}