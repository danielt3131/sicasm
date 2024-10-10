/**
 * @author Daniel J. Thompson (N01568044)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"



int main(int argc, char **argv) {
    if (argc != 2) {
        printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
        return (EXIT_FAILURE);
    }
    //FILE *fp = fopen("copymystring.sic", "r");
    FILE *fp = fopen(argv[1], "r");
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
    struct symbolTable *symbolTable = createSymbolTable(fp);
    fclose(fp);
    if (symbolTable == NULL) {
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < symbolTable->numberOfSymbols; i++) {
        printf("%s \t %X\n", symbolTable->symbols[i].name, symbolTable->symbols[i].address);
    }
    freeSymbolTable(symbolTable);
    return (EXIT_SUCCESS);
}