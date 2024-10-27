#include "fileBuffer.h"
#include <stdlib.h>
#include <string.h>

/**
 * Saves the contents of a file into memory and converts CRLF to LF
 * @param sourceFile The file stream to read from
 * @return A file buffer containing all non comment lines
 */
fileBuffer* createFileBuffer(FILE *sourceFile) {
    fileBuffer *buffer = malloc(sizeof(fileBuffer));
    buffer->allocationAmount = 10;
    buffer->numLines = 0;
    buffer->lines = malloc(buffer->allocationAmount * sizeof(char *));
    buffer->lineNumbers = malloc(buffer->allocationAmount * sizeof(int));

    char *line = NULL;
    size_t size = 0;
    int bufferSize = 0;
    int i = 1;
    while ((bufferSize = getline(&line, &size, sourceFile)) != -1) {
        if (line[0] != '#') {
            if (buffer->numLines >= buffer->allocationAmount) {
                buffer->allocationAmount *= 2;
                buffer->lines = realloc(buffer->lines, buffer->allocationAmount * sizeof(char *));
                buffer->lineNumbers = realloc(buffer->lineNumbers, buffer->allocationAmount * sizeof(int));
            }
            buffer->lines[buffer->numLines] = malloc(bufferSize + 1);
            strcpy(buffer->lines[buffer->numLines], line);
            if (buffer->lines[buffer->numLines][bufferSize - 2] == '\r') {
                buffer->lines[buffer->numLines][bufferSize - 2] = '\n';
                buffer->lines[buffer->numLines][bufferSize - 1] = '\0';
            }
            buffer->lineNumbers[buffer->numLines] = i;
            buffer->numLines++;
        }
        i++;
    }
    free(line);
    // Shrink to amount needed
    buffer->allocationAmount = buffer->numLines;
    buffer->lines = realloc(buffer->lines, buffer->allocationAmount * sizeof(char *));
    buffer->lineNumbers = realloc(buffer->lineNumbers, buffer->allocationAmount * sizeof(int));
    fclose(sourceFile);
    return buffer;
}


void freeFileBuffer(fileBuffer *buffer) {
    for (int i = 0; i < buffer->numLines; i++) {
        free(buffer->lines[i]);
    }
    free(buffer->lineNumbers);
    free(buffer->lines);
    free(buffer);
}