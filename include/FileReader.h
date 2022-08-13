#ifndef FILE_READER_H
#define FILE_READER_H
#include <mpi.h>

enum Channel
{
	R = 0,
	G = 1,
	B = 2,
};

typedef struct
{
	int channels[3];
} Color;

MPI_Datatype getColorDatatype();
#define COLOR_TYPE getColorDatatype()

typedef struct
{
	int x;
	int y;
	Color color;
} FixedPoint;

MPI_Datatype getFixedPointDatatype();
#define FIXED_POINT_TYPE getFixedPointDatatype()

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