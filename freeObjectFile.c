#include "freeObjectFile.h"

void freeObjectFile(objectFile * objFile) {
    for (int i = 0; i < objFile->tRecords->numStrings; i++) {
        free(objFile->tRecords->stringArray[i]);
    }

    for (int i = 0; i < objFile->mRecords->numStrings; i++) {
        free(objFile->mRecords->stringArray[i]);
    }
    free(objFile->tRecords->stringArray);
    free(objFile->tRecords);
    free(objFile->mRecords->stringArray);
    free(objFile->mRecords);
    free(objFile->hRecord);
    free(objFile->eRecord);
    free(objFile);
}