#ifndef ITERATION_H
#define ITERATION_H

#include "Color.h"

#define ITERATION_COUNT 10000

/* This struct hosts all data necessary for applying the stencil to an image, namely:
 * - The dimensions of the image ([h]eight, [w]idth)
 * - The contents of the image, pointed to by 'image'
 * - The 2 arrays which must be used for calculations of the edge pixels.
 *   - the array on index 0 contains the pixels from the 'top' neighbor, and
 *   - the array on index 1, the bottom neighbor
 */
typedef struct
{
	int h;
	int w;
	Color* image;
	Color* adj[2];
} IterationData;

void doStencilIteration(const IterationData d);

#endif // ITERATION_H
