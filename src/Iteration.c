#include <string.h>

#include "../include/Iteration.h"
#include "../include/Communication.h"

Color getPixelAt(int i, int j, const IterationData d)
{
	if (i < 0)
		return d.adj[TOP][j];
	if (i >= d.h)
		return d.adj[BOTTOM][j];

	if (j < 0 || j >= d.w)
		return GRAY;

	return d.image[i * d.w + j];
}

Color averageColorOf(const Color colors[const 5])
{
	Color result = WHITE;

	for (int i = 0; i < 5; ++i)
	{
		for (int c = R; c <= B; ++c)
			result.channels[c] += colors[i].channels[c];
	}

	for (int c = R; c <= B; ++c)
		result.channels[c] /= 5;

	return result;
}

#define ARR_LEN(A) (sizeof (A) / sizeof *(A))

void doStencilIteration(const IterationData d)
{
	const static int stencilOffsets[][2] = {
		{ 0, 0, }, { -1, 0, }, { +1, 0, }, { 0, -1, }, { 0, +1, },
	};

	Color newImage[d.h][d.w];

	for (int i = 0; i < d.h; ++i)
	{
		for (int j = 0; j < d.w; ++j)
		{
			Color stencilPixels[ARR_LEN(stencilOffsets)] = {
				[0] = d.image[i * d.w + j],
			};

			// Skip self (0, 0) since we know it is going to be inbounds
			for (int p = 1; p < ARR_LEN(stencilOffsets); ++p)
			{
				stencilPixels[p] = getPixelAt(
					i + stencilOffsets[p][0],
					j + stencilOffsets[p][1],
					d
				);
			}

			newImage[i][j] = averageColorOf(stencilPixels);
		}
	}

	memcpy(d.image, newImage, sizeof newImage);
}