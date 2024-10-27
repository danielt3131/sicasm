#include <stdio.h>
#include "tables.h"
#ifndef FILEBUFFER_H
#define FILEBUFFER_H
fileBuffer* createFileBuffer(FILE *sourceFile);
void freeFileBuffer(fileBuffer *buffer);
#endif
