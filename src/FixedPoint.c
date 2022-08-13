#include <stdio.h>
#include <stdlib.h>

#include "../include/FixedPoint.h"

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