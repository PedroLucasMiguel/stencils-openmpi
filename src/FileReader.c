#include <stdio.h>
#include <stdlib.h>

#include "../include/Color.h"
#include "../include/FileReader.h"

static int readIntFromFile(FILE* const f)
{
	int i;
	fscanf(f, "%d", &i);
	return i;
}



static FixedPoint* readFixedPointsFromFile(FILE* const f, int const count)
{
	FixedPoint* const fixedPoints = malloc(count * sizeof *fixedPoints);

	for (int i = 0; i < count; ++i)
	{
		fixedPoints[i] = (FixedPoint){
			.x = readIntFromFile(f),
			.y = readIntFromFile(f),
			.color = readColorFromFile(f),
		};
	}

	return fixedPoints;
}

ImageData readImageFile(const char* const path)
{
	FILE* const f = fopen(path, "r");

	if (f == NULL)
	{
		perror("Unable to open file");
		exit(EXIT_FAILURE);
	}

	const int size = readIntFromFile(f);
	const int fixedPointCount = readIntFromFile(f);

	const ImageData imageData = (ImageData){
		.size = size,
		.fixedPointCount = fixedPointCount,
		.fixedPoints = readFixedPointsFromFile(f, fixedPointCount),
	};

	fclose(f);

	return imageData;
}



void printFixedPoint(FILE* const out, const FixedPoint fp)
{
	fprintf(out, "(%d, %d): ", fp.x, fp.y);
	printColor(out, fp.color);
	fprintf(out, "\n");
}

void printImageData(FILE* const out, const ImageData data)
{
	printf("%d x %d Image; %d Fixed Points:\n",
		data.size, data.size, data.fixedPointCount);

	for (int i = 0; i < data.fixedPointCount; i++)
		printFixedPoint(out, data.fixedPoints[i]);
}

MPI_Datatype getFixedPointDatatype()
{
	static MPI_Datatype committedDatatype = MPI_DATATYPE_NULL;

	if (committedDatatype != MPI_DATATYPE_NULL)
		return committedDatatype;

	MPI_Type_create_struct(3, (int[]){ 1, 1, 1 }, (MPI_Aint[]){
		offsetof(FixedPoint, x),
		offsetof(FixedPoint, y),
		offsetof(FixedPoint, color),
	}, (MPI_Datatype[]){ MPI_INT, MPI_INT, COLOR_TYPE, }, &committedDatatype);

	MPI_Type_commit(&committedDatatype);

	return committedDatatype;
}