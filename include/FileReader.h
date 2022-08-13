#ifndef FILE_READER_H
#define FILE_READER_H
#include <mpi.h>

typedef struct
{
	int r;
	int g;
	int b;
} Color;

MPI_Datatype getColorDatatype();
#define COLOR_TYPE getColorDatatype()

typedef struct
{
	int x;
	int y;
	Color color;
} FixedPoint;

typedef struct
{
	int size;
	int fixedPointCount;
	FixedPoint* fixedPoints;
} ImageData;

ImageData readImageFile(const char* path);
void printImageData(FILE* out, ImageData data);
void printFixedPoint(FILE* out, FixedPoint fp);
void printColor(FILE* out, Color c);

#endif