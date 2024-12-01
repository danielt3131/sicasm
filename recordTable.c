/**
 * @author Tianyu Wu
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "tables.h"
#include "stdlib.h"
#include "string.h"

struct stringArray* printRecordTable(recordList table) {
    struct stringArray* array = malloc(sizeof(struct stringArray));
    array->numStrings = 0;
    array->allocatedAmount = 4;
    array->stringArray = malloc(array->allocatedAmount * sizeof(char *));
    record* temp;
    temp = table.head;
    int i = 0;
    while(temp != NULL)   {
        if (array->numStrings >= array->allocatedAmount) {
            array->allocatedAmount *= 2;
            array->stringArray = realloc(array->stringArray, array->allocatedAmount * sizeof(char *));
        }
        array->stringArray[i] = malloc(82);
        sprintf(array->stringArray[i], "%s\n", temp->r);
        temp = temp->next;
        i++;
        array->numStrings++;
    }
    return array;
}

void insertRecord(recordList* table, char* r) {
    record* new = malloc(sizeof(record));
    memset( new, '\0', sizeof(record));
    new->r = r;

    if(table->head != NULL) {   
        table->tail->next = new;
        table->tail = new;
    }
    else {
        new->next = NULL;
        table->head = new;
        table->tail = new;
    }
}


void freeRecord(recordList* table) {
    record* temp;
    while(table->head != NULL) {
        temp = table->head->next;
        free(table->head->r);
        free(table->head);
        table->head = temp;
    }

    free(table);
}