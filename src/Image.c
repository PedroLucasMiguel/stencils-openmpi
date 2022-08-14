#include <stdio.h>
#include <stdlib.h>

#include "../include/Image.h"

/* Iterates over the fixed points in 'data' and set the appropriate pixels in the image */
void setFixedPointsOnImageSlice(const ImageData data, Color (* image)[data.size])
{
	for (int i = 0; i < data.fixedPointCount; ++i)
	{
		const FixedPoint fp = data.fixedPoints[i];
		image[fp.i][fp.j] = fp.color;
	}
}

/* Reads the image data contained in the file pointed by 'path' and collects it into the appropriate structure. */
ImageData readImageData(const char* const path)
{
	FILE* const f = fopen(path, "r");

	if (f == NULL)
	{
		perror("Unable to open file");
		exit(EXIT_FAILURE);
	}

	ImageData imageData;

	fscanf(f, "%d %d", &imageData.size, &imageData.fixedPointCount);
	imageData.fixedPoints = readFixedPointsFromFile(f, imageData.fixedPointCount);

	fclose(f);

	return imageData;
}

void printImageData(FILE* const out, const ImageData data)
{
	printf("%d i %d Image; %d Fixed Points:\n",
		data.size, data.size, data.fixedPointCount);

	for (int i = 0; i < data.fixedPointCount; i++)
		printFixedPoint(out, data.fixedPoints[i]);
}

static void printImageLine(FILE* const out, const int len, const Color image[const len])
{
	for (int i = 0; i < len; ++i)
	{
		printColor(out, image[i]);
		putc(' ', out);
	}
	putc('\n', out);
}

void printImage(FILE* const out, const int height, const int width, const Color image[const height][width])
{
	for (int i = 0; i < height; ++i)
	{
		printImageLine(out, width, image[i]);
	}
}

/* Removes from imageData fixedPoints which i coordinate is not in the interval [ start, end ).
 * Also subtracts 'start' from the 'i' coordinate of each point.
 */
void transformFixedPoints(ImageData* imageData, int start, int end)
{
	FixedPoint* filtered = malloc(imageData->fixedPointCount * sizeof *filtered);
	int filteredCount = 0;

	for (int i = 0; i < imageData->fixedPointCount; ++i)
	{
		const FixedPoint fp = imageData->fixedPoints[i];

		if (fp.i < start || fp.i >= end)
			continue;

		filtered[filteredCount] = (FixedPoint){
			.i = fp.i - start, .j = fp.j,
			.color = fp.color,
		};

		filteredCount++;
	}

	free(imageData->fixedPoints);

	imageData->fixedPointCount = filteredCount;
	imageData->fixedPoints = filtered;
}