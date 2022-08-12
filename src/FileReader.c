#include <stdio.h>
#include <stdlib.h>

#include "../include/FileReader.h"

MPI_Datatype addFixedPointToMpi()
{
// TODO: Use XMacros to avoid errors
	MPI_Aint offsets[] = {
		offsetof(FixedPoint, x),
		offsetof(FixedPoint, y),
		offsetof(FixedPoint, r),
		offsetof(FixedPoint, g),
		offsetof(FixedPoint, b),
	};

	MPI_Datatype mpi_FixedPoint;

	MPI_Type_create_struct(5,
		(int[]){ [0 ... 4]=1 },
		offsets,
		(MPI_Datatype[]){ [0 ... 4]=MPI_INT },
		&mpi_FixedPoint
	);

	MPI_Type_commit(&mpi_FixedPoint);

	return mpi_FixedPoint;
}

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
			.r = readIntFromFile(f),
			.g = readIntFromFile(f),
			.b = readIntFromFile(f),
		};
	}

	return fixedPoints;
}

ImageData readImageFile(const char* const path)
{
	FILE* const f = fopen(path, "r");

	if (f == NULL)
	{
		perror("Unable to open file.");
		exit(1);
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

void printFixedPoint(const FixedPoint* const fp)
{
	printf("X: %d || Y: %d || R: %d || G: %d || B: %d\n",
		fp->x, fp->y,
		fp->r, fp->g, fp->b
	);
}

void printImageData(const ImageData* const data)
{
	printf("Image size: %d | Qtd.Fixed Points: %d\n", data->size, data->fixedPointCount);

	for (int i = 0; i < data->fixedPointCount; i++)
		printFixedPoint(&data->fixedPoints[i]);
}
