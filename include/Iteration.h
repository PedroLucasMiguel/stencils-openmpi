#ifndef ITERATION_H
#define ITERATION_H

#include "Color.h"

#define ITERATION_COUNT 10000

typedef struct
{
	int h;
	int w;
	Color* image;
	Color* adj[2];
} IterationData;

void doStencilIteration(const IterationData d);

#endif // ITERATION_H
