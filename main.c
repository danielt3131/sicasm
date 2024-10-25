/**
 * @author Daniel J. Thompson (N01568044)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "createObjectFile.h"
#include "freeObjectFile.h"

int main(int argc, char **argv) {

    if (!(argc < 4 && argc > 1)) {
        printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
        return (EXIT_FAILURE);
    }
    //FILE *sourceFile = fopen("copymystring.sic", "r");
    FILE *sourceFile = fopen(argv[1], "r");
    if (sourceFile == NULL) {
        printf("The file %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }
    /**
     * @brief Pass 1
     */
    struct symbolTable *symbolTable = createSymbolTable(sourceFile);
    fclose(sourceFile);
    if (symbolTable == NULL) {
        return EXIT_FAILURE;
    }
    /**
     * Pass 1 only option
     */
    if (argc == 3 && strcmp(argv[2], "--pass1only") == 0) {
        for (int i = 0; i < symbolTable->numberOfSymbols; i++) {
            printf("%s \t %X\n", symbolTable->symbols[i].name, symbolTable->symbols[i].address);
        }
        freeSymbolTable(symbolTable);
        return EXIT_SUCCESS;
    }
    sourceFile = fopen(argv[1], "r");
    /**
     * Pass 2
     */
    objectFile *objFile = createObjectFile(symbolTable, sourceFile);
    
    if (objFile == NULL) {
        return EXIT_FAILURE;
    } 

    char *outputFilename = malloc(strlen(argv[1]) + 8);
    strcpy(outputFilename, argv[1]);
    strcat(outputFilename, ".obj");
    FILE *outputFile = fopen(outputFilename, "w");
    fprintf(outputFile, "%s", objFile->hRecord);
    for (int i = 0; i < objFile->tRecords->numStrings; i++) {
        fprintf(outputFile, "%s", objFile->tRecords->stringArray[i]);
    }
    fprintf(outputFile, "%s", objFile->eRecord);
    for (int i = 0; i < objFile->mRecords->numStrings; i++) {
        fprintf(outputFile, "%s", objFile->mRecords->stringArray[i]);
    }
    fclose(outputFile);
    free(outputFilename);
    freeObjectFile(objFile);
    freeSymbolTable(symbolTable);
}
