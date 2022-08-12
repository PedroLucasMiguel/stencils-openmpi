#include "../include/FileReader.h"
#include <stdlib.h>
#include <stdio.h>

void printFileData(FileData* data) {
    printf("Image size: %d | Qtd.Fixed Points: %d\n", data->imageSize, data->qtdFixedPoints);

    for(int i = 0; i < data->qtdFixedPoints; i++) {
        printf("X: %d || Y: %d || R: %d || G: %d || B: %d\n", data->fixedPoints[i][0], 
                                                              data->fixedPoints[i][1], 
                                                              data->fixedPoints[i][2], 
                                                              data->fixedPoints[i][3], 
                                                              data->fixedPoints[i][4]);
    }
}

void readFile(char* path, FileData* data) {
    FILE* f = fopen(path, "r");

    fscanf(f, "%d %d", &(data)->imageSize, &(data)->qtdFixedPoints);

    data->fixedPoints = malloc(data->qtdFixedPoints * sizeof(int[5]));
    
    for(int i = 0; i < data->qtdFixedPoints; i++) {
        fscanf(f, "%d %d %d %d %d", &data->fixedPoints[i][0], 
                                    &data->fixedPoints[i][1], 
                                    &data->fixedPoints[i][2],
                                    &data->fixedPoints[i][3],
                                    &data->fixedPoints[i][4]);
    }
}