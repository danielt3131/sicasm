/**
 * @author Daniel J. Thompson (N01568044)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "freeObjectFile.h"

void freeObjectFile(objectFile * objFile) {
    for (int i = 0; i < objFile->tRecords->numStrings; i++) {
        free(objFile->tRecords->stringArray[i]);
    }
    if (objFile->mRecords->numStrings > 0) {
        for (int i = 0; i < objFile->mRecords->numStrings; i++) {
            free(objFile->mRecords->stringArray[i]);
        }
        free(objFile->mRecords->stringArray);
    }
    if (objFile->dRecords != NULL) {
        for (int i = 0; i < objFile->dRecords->numStrings; i++) {
            free(objFile->dRecords->stringArray[i]);
        }
        free(objFile->dRecords->stringArray);
        free(objFile->dRecords);
    }
    free(objFile->mRecords);
    free(objFile->tRecords->stringArray);
    free(objFile->tRecords);
    free(objFile->hRecord);
    free(objFile->eRecord);
    free(objFile);
}