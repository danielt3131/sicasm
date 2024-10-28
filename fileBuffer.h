#include <stdio.h>
#include "tables.h"
#ifndef FILEBUFFER_H
#define FILEBUFFER_H
fileBuffer* createFileBuffer(FILE *sourceFile, int *numberOfSymbols);
void freeFileBuffer(fileBuffer *buffer);
#endif
