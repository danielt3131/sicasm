#include "tables.h"

void printRecordTable(recordList table) {
    record* temp;
    temp = table.head;
    while(temp != NULL)   {
        printf("%s\n", temp->r);
        temp = temp->next;
    }
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