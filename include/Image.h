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

void setFixedPointsOnImageSlice(const ImageData data, Color (* image)[data.size]);

ImageData readImageData(const char* path);
void printImageData(FILE* out, ImageData data);

void printImage(FILE* out, int height, int width, const Color image[height][width]);

void filterFixedPoints(ImageData* imageData, int start, int end);

#endif