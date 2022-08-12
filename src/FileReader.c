#include "../include/FileReader.h"
#include <stdlib.h>
#include <stdio.h>

void printFileData(FileData* data) {
    printf("Image size: %d | Qtd.Fixed Points: %d\n", data->imageSize, data->qtdFixedPoints);

    for(int i = 0; i < data->qtdFixedPoints; i++) {
        printf("X: %d || Y: %d || R: %d || G: %d || B: %d\n", data->fixedPoints[i].x, 
                                                              data->fixedPoints[i].y, 
                                                              data->fixedPoints[i].r, 
                                                              data->fixedPoints[i].g, 
                                                              data->fixedPoints[i].b);
    }
}

void readFile(char* path, FileData* data) {
    FILE* f = fopen(path, "r");

    if (f == NULL) {
        printf("\nFILE NOT FOUND/COUL NOT BE READED\n");
        return;
    }

    fscanf(f, "%d %d", &data->imageSize, &data->qtdFixedPoints);

    data->fixedPoints = malloc(data->qtdFixedPoints * sizeof(FixedPoint));
    
    for(int i = 0; i < data->qtdFixedPoints; i++) {
        fscanf(f, "%d %d %d %d %d", &data->fixedPoints[i].x, 
                                    &data->fixedPoints[i].y, 
                                    &data->fixedPoints[i].r,
                                    &data->fixedPoints[i].g,
                                    &data->fixedPoints[i].b);
    }

    fclose(f);
}