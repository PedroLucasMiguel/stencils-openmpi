#ifndef FILE_READER_H
#define FILE_READER_H

#include <mpi.h>

#include "FixedPoint.h"

typedef struct
{
	int size;
	int fixedPointCount;
	FixedPoint* fixedPoints;
} ImageData;

ImageData readImageDataFile(const char* path);
void printImageData(FILE* out, ImageData data);

#endif