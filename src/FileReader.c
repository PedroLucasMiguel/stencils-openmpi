#include "../include/FileReader.h"
#include <stdlib.h>
#include <stdio.h>

MPI_Datatype addFixedPointToMpi() {
    /* create a type for struct car */
    const int nitems = 5;
    int blocklengths[5] = {1,1,1,1,1};
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Datatype mpi_FixedPoint;
    MPI_Aint offsets[5];

    offsets[0] = offsetof(FixedPoint, x);
    offsets[1] = offsetof(FixedPoint, y);
    offsets[2] = offsetof(FixedPoint, r);
    offsets[3] = offsetof(FixedPoint, g);
    offsets[4] = offsetof(FixedPoint, b);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_FixedPoint);
    MPI_Type_commit(&mpi_FixedPoint);

    return mpi_FixedPoint;
}

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