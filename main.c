/**
 * @author Daniel J. Thompson (N01568044)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fileBuffer.h"
#include "tables.h"
#include "createXeObjectFile.h"
#include "createSicObjectFile.h"
#include "freeObjectFile.h"

void printHelpMenu();
int main(int argc, char **argv) {
    bool printMode = false;
    bool isPiped = false;
    FILE *sourceFile;
    if (!isatty(STDIN_FILENO)) {
        sourceFile = stdin;
        printMode = true;
        isPiped = true;
    }

    if (!(argc < 5 && argc > 1) && !printMode) {
        printf("SIC and SIC/XE assembler by Daniel J. Thompson, Tianyu Wu, Samuel Gray\n");
        printf("USAGE: %s <filename, - where filename is a SIC or SIC/XE Assembly File\n", argv[0]);
        printf("Run %s -h for more info\n", argv[0]);
        return (EXIT_FAILURE);
    }
    // Prints help menu
    if (argc == 2) {
        if (!strcmp("-h", argv[1])) {
            printf("USAGE: %s <filename, - where filename is a SIC Assembly File\n", argv[0]);
            printHelpMenu();
            return EXIT_SUCCESS;
        }
        if (!strcmp("-v", argv[1])) {
            printf("Version 2 by Daniel J. Thompson, Tianyu Wu, Samuel Gray\n");
            printf("Licensed under the GPLv2 or Later");
            return EXIT_SUCCESS;
        }
    }
    // Support piping from other processes by reading in stdin to a tmp file.
    if (!printMode) {
        sourceFile = fopen(argv[1], "r");
    }
    if (sourceFile == NULL) {
        // Tests for read permissions by if the file exists
         if (access(argv[1], F_OK)) {
            fprintf(stderr, "The file %s does not exist\n", argv[1]);
         } else {
             fprintf(stderr, "The user %s lacks read permissions for %s\n", getlogin(), argv[1]);
         }
         return EXIT_FAILURE;
    }
    // Create file buffer
    int numSymbols = 0;
    fileBuffer *buffer = createFileBuffer(sourceFile, &numSymbols);
    /**
     * @brief Pass 1
     */
    bool isXE = false;
    struct symbolTable *symbolTable = createSymbolTable(buffer, &numSymbols, &isXE);
    if (symbolTable == NULL) {
        freeFileBuffer(buffer);
        //fclose(sourceFile);
        return EXIT_FAILURE;
    }
    /**
     * Pass 1 only option
     */
    if (argc < 4 && !strcmp(argv[argc - 1], "--pass1only")) {
        for (int i = 0; i < symbolTable->numberOfSymbols; i++) {
            printf("%s \t %X\n", symbolTable->symbols[i].name, symbolTable->symbols[i].address);
        }
        freeSymbolTable(symbolTable);
        freeFileBuffer(buffer);
        return EXIT_SUCCESS;
    }
    /**
     * Pass 2
     * If isXE is true then handle
     */
    objectFile *objFile;
    if (isXE) {
        objFile = createXeObjectFile(symbolTable, buffer);
    } else {
        objFile = createSicObjectFile(symbolTable, buffer);
    }

    freeFileBuffer(buffer);
    if (objFile == NULL) {
        freeSymbolTable(symbolTable);
        return EXIT_FAILURE;
    }

    char *outputFilename;
    bool savePath = false;
    if (argc > 2 && !strcmp(argv[2], "-o")) {
        outputFilename = argv[3];
        savePath = true;
    } else if (!isPiped) {
        outputFilename = malloc(strlen(argv[1]) + 8);
        strcpy(outputFilename, argv[1]);
        strcat(outputFilename, ".obj");
    }
    FILE *outputFile;
    if (printMode || (argc > 2 && !strcmp("-p", argv[2]))) {
        outputFile = stdout;
        printMode = true;
    } else {
        outputFile = fopen(outputFilename, "w");
    }
    if (outputFile == NULL) {
        fprintf(stderr, "Unable to write to %s\n", outputFilename);
        fprintf(stderr, "You can either run me as root, fix the file permissions at %s for user %s, or run with -p to print the object file to stdout and you handle the pipe redirection\n", outputFilename, getlogin());
        free(outputFilename);
        freeSymbolTable(symbolTable);
        freeObjectFile(objFile);
        fclose(outputFile);
        return EXIT_FAILURE;
    }
    fprintf(outputFile, "%s", objFile->hRecord);
    for (int i = 0; i < objFile->tRecords->numStrings; i++) {
        fprintf(outputFile, "%s", objFile->tRecords->stringArray[i]);
    }
    fprintf(outputFile, "%s", objFile->eRecord);
    if (objFile->mRecords->numStrings > 0) {
        for (int i = 0; i < objFile->mRecords->numStrings; i++) {
            fprintf(outputFile, "%s", objFile->mRecords->stringArray[i]);
        }
    }
    if (!printMode) {
        fclose(outputFile);
    }
    if (!(savePath || isPiped)) {
        free(outputFilename);
    }
    freeObjectFile(objFile);
    freeSymbolTable(symbolTable);
}


void printHelpMenu() {
    /**
     * Get the current working directory from getcwd syscall
     * The arguments NULL and 0 are there to have the function wrapper for the syscall to automatically allocate the nesscarry amount of memory
     */
    char *workingDirectory = getcwd(NULL, 0);
    printf("CLI arguments:\n\t--pass1only will only print the symbol table of the assembly file\n"
           "\t-o will save the object file to the specified location instead of %s/example.sic.obj\n"
           "\t-p will print the contents of the object file to stdout and will not create a file (used for pipes and redirection)\n"
           "\t-v displays version info\n\t-h display this help menu\n", workingDirectory);
    free(workingDirectory);
}