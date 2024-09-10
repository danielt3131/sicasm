#ifndef TABLES_H
#define TABLES_H
typedef struct {
    char *name;
    int address;
    int lineNumber;
} symbol;

struct symbolTable {
    symbol *symbols;
    int numberOfSymbols;
    int allocatedAmount;
};
#endif