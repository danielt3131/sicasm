/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <stdio.h>
#include "tables.h"
#ifndef FILEBUFFER_H
#define FILEBUFFER_H
fileBuffer* createFileBuffer(FILE *sourceFile, int *numberOfSymbols);
void freeFileBuffer(fileBuffer *buffer);
void replaceWhitespace(char *str, int size);
#endif
