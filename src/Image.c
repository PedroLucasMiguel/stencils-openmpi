#include <stdio.h>
#include <stdlib.h>

#include "../include/Color.h"
#include "../include/Image.h"

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