#include <stdio.h>
#include <stdlib.h>

#include "../include/FixedPoint.h"

void printFixedPoint(FILE* const out, const FixedPoint fp)
{
	fprintf(out, "(%d, %d): ", fp.x, fp.y);
	printColor(out, fp.color);
	fprintf(out, "\n");
}

FixedPoint* readFixedPointsFromFile(FILE* const f, int const count)
{
	FixedPoint* const fixedPoints = malloc(count * sizeof *fixedPoints);

	for (int i = 0; i < count; ++i)
	{
		fscanf(f, "%d %d", &fixedPoints[i].x, &fixedPoints[i].y);
		fixedPoints[i].color = readColorFromFile(f);
	}

	return fixedPoints;
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