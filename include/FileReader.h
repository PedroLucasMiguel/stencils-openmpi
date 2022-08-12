#ifndef FILE_READER_H
#define FILE_READER_H
#include <mpi.h>
typedef struct FixedPointsData {
    int x;
    int y;
    int r;
    int g;
    int b;
}FixedPoint;

typedef struct FileReaderData {
    int imageSize;
    int qtdFixedPoints;
    FixedPoint* fixedPoints;
}FileData;

MPI_Datatype addFixedPointToMpi();
void readFile(char* path, FileData* data);
void printFileData(FileData* data);

#endif