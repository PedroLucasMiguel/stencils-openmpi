#include <stdio.h>
#include <stdlib.h>

#include "../include/Image.h"

void setFixedPointsOnImageSlice(const ImageData data, Color (* image)[data.size])
{
	for (int i = 0; i < data.fixedPointCount; ++i)
	{
		const FixedPoint fp = data.fixedPoints[i];
		image[fp.x][fp.y] = fp.color;
	}
}

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
	printf("%d x %d Image; %d Fixed Points:\n",
		data.size, data.size, data.fixedPointCount);

	for (int i = 0; i < data.fixedPointCount; i++)
		printFixedPoint(out, data.fixedPoints[i]);
}

static void printImageLine(FILE* const out, const int len, const Color image[len])
{
	for (int i = 0; i < len; ++i)
	{
		printColor(out, image[i]);
		putc(' ', out);
	}
	putc('\n', out);
}

void printImage(FILE* const out, const int height, const int width, const Color image[height][width])
{

	for (int i = 0; i < height; ++i)
	{
		printImageLine(out, width, image[i]);
	}
}

/*
 * Filters from imageData fixedPoints that are not in [start, end), also subtracting
 * 'start' from the 'x' coordinate of each point.
 */
void filterFixedPoints(ImageData* const imageData, const int start, const int end)
{
	FixedPoint* filtered = malloc(imageData->fixedPointCount * sizeof *filtered);
	int filteredCount = 0;

	for (int i = 0; i < imageData->fixedPointCount; ++i)
	{
		const FixedPoint fp = imageData->fixedPoints[i];

		if (fp.x < start || fp.x >= end)
			continue;

		filtered[filteredCount] = (FixedPoint){
			.x = fp.x - start, .y = fp.y,
			.color = fp.color,
		};

		filteredCount++;
	}

	free(imageData->fixedPoints);

	imageData->fixedPointCount = filteredCount;
	imageData->fixedPoints = filtered;
}