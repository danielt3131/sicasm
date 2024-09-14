#include "tables.h"
#include <stdlib.h>

void freeSymbolTable(struct symbolTable *symbolTable) {
    for (int i = 0; i < symbolTable->numberOfSymbols; i++) {
        free(symbolTable->symbols[i].name);
    }
    free(symbolTable->symbols);
    free (symbolTable);
}