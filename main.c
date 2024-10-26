/**
 * @author Daniel J. Thompson (N01568044)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables.h"
#include "createObjectFile.h"
#include "freeObjectFile.h"

void printHelpMenu();
int main(int argc, char **argv) {
    if (!(argc < 5 && argc > 1)) {
        printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
        printf("Run %s -h for more info\n", argv[0]);
        return (EXIT_FAILURE);
    }

    // Prints help menu
    if (argc == 2) {
        if (!strcmp("-h", argv[1])) {
            printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
            printHelpMenu(argv[0]);
            return EXIT_SUCCESS;
        }
        if (!strcmp("-v", argv[1])) {
            printf("Version 1.0 by Daniel J. Thompson\n");
            return EXIT_SUCCESS;
        }
    }
    //FILE *sourceFile = fopen("copymystring.sic", "r");
    FILE *sourceFile = fopen(argv[1], "r");
    if (sourceFile == NULL) {
        printf("The file %s does not exist or insufficient permissions to read in %s\n", argv[1], argv[1]);
        return EXIT_FAILURE;
    }
    /**
     * @brief Pass 1
     */
    struct symbolTable *symbolTable = createSymbolTable(sourceFile);
    if (symbolTable == NULL) {
        fclose(sourceFile);
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
        fclose(sourceFile);
        return EXIT_SUCCESS;
    }
    // Rewinds the file pointer back to the beginning of the file
    rewind(sourceFile);
    /**
     * Pass 2
     */
    objectFile *objFile = createObjectFile(symbolTable, sourceFile);
    fclose(sourceFile);
    if (objFile == NULL) {
        return EXIT_FAILURE;
    }

    char *outputFilename;
    bool savePath = false;
    if (argc > 2 && !strcmp(argv[2], "-o")) {
        outputFilename = argv[3];
        savePath = true;
    } else {
        outputFilename = malloc(strlen(argv[1]) + 8);
        strcpy(outputFilename, argv[1]);
        strcat(outputFilename, ".obj");
    }
    bool printMode = false;
    FILE *outputFile;
    if (argc > 2 && !strcmp("-p", argv[2])) {
        outputFile = stdout;
        printMode = true;
    } else {
        outputFile = fopen(outputFilename, "w");
    }
    if (outputFile == NULL) {
    fprintf(stderr, "Unable to write to %s\n", outputFilename);
    fprintf(stderr, "You can either run me as root, fix the file permissions at %s, or run with -p to print the object file to stdout and you handle the pipe redirection\n", outputFilename);
    free(outputFilename);
        freeSymbolTable(symbolTable);
        freeObjectFile(objFile);
        return EXIT_FAILURE;
    }
    fprintf(outputFile, "%s", objFile->hRecord);
    for (int i = 0; i < objFile->tRecords->numStrings; i++) {
        fprintf(outputFile, "%s", objFile->tRecords->stringArray[i]);
    }
    fprintf(outputFile, "%s", objFile->eRecord);
    for (int i = 0; i < objFile->mRecords->numStrings; i++) {
        fprintf(outputFile, "%s", objFile->mRecords->stringArray[i]);
    }
    if (!printMode) {
        fclose(outputFile);
    }
    if (!savePath) {
        free(outputFilename);
    }
    freeObjectFile(objFile);
    freeSymbolTable(symbolTable);
}

void printHelpMenu() {
    // Opens up a pipe to get the output of pwd (print working directory)
    FILE *pipe = popen("pwd", "r");
    char buffer[200];
    fgets(buffer, 200, pipe);
    fclose(pipe);
    buffer[strlen(buffer) - 1] = '\0'; // Remove LF
    printf("CLI arguments:\n\t--pass1only will only print the symbol table of the assembly file\n"
           "\t-o will save the object file to the specified location instead of %s/example.sic.obj\n"
           "\t-p will print the contents of the object file to stdout and will not create a file (used for pipes and redirection)\n"
           "\t-v displays version info\n\t-h display this help menu\n", buffer);
}
