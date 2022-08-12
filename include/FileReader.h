#ifndef FILE_READER_H
#define FILE_READER_H
#include <mpi.h>

typedef struct
{
	int x, y;
	int r, g, b;
} FixedPoint;

typedef struct
{
	const int size;
	const int fixedPointCount;
	const FixedPoint* fixedPoints;
} ImageData;

ImageData readImageFile(const char* path);
void printImageData(const ImageData* data);
MPI_Datatype addFixedPointToMpi();

#endif