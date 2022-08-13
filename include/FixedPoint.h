#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdio.h>
#include <mpi.h>

#include "Color.h"

typedef struct
{
	int x;
	int y;
	Color color;
} FixedPoint;

MPI_Datatype getFixedPointDatatype();
#define FIXED_POINT_TYPE getFixedPointDatatype()

void printFixedPoint(FILE* out, FixedPoint fp);

// Note: this function returns a pointer that must be deallocated with free
__attribute__((malloc))
FixedPoint* readFixedPointsFromFile(FILE* f, int count);

#endif // FIXEDPOINT_H
