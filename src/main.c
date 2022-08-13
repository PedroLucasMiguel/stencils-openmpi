#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "../include/FileReader.h"

enum MessageTags
{
	TAG_ANY = 1,
	TAG_COMM_UP,
	TAG_COMM_DOWN,
};

__attribute__((format(printf, 2, 3)))
void debug_print(const int rank, const char* const fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("%d: ", rank);
	vprintf(fmt, args);

	va_end(args);
}

int getNumProcesses(MPI_Comm comm)
{
	int numProcesses;
	MPI_Comm_size(comm, &numProcesses);
	return numProcesses;
}

MPI_Comm arrangeProcesses(int numProcesses)
{
	MPI_Comm newComm;
	MPI_Cart_create(MPI_COMM_WORLD, 1, (int[]){ numProcesses }, (int[]){ false }, true, &newComm);
	return newComm;
}

int getProcessRank(MPI_Comm comm)
{
	int myRank;
	MPI_Comm_rank(comm, &myRank);
	return myRank;
}

typedef struct
{
	int up;
	int down;
} Neighbors;

#define COORDINATOR_RANK 0

#define IF_COORDINATOR(RANK, CODE) \
    if ((RANK) == COORDINATOR_RANK) {\
        do {                  \
            CODE              \
        } while (0);          \
    }0

#define GRAY (Color){ 127, 127, 127, }

#define WHITE (Color){ 0, 0, 0, }

#define ITERATION_COUNT 10000

Neighbors getNeighbors(MPI_Comm comm)
{
#define LINEAR 0
#define ADJACENT 1
	int up;
	int down;
	MPI_Cart_shift(comm, LINEAR, ADJACENT, &up, &down);
	return (Neighbors){ up, down };
}

void start_procedure(MPI_Comm comm, int numProcesses);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	const int numProcesses = getNumProcesses(MPI_COMM_WORLD);
	MPI_Comm newComm = arrangeProcesses(numProcesses);

	start_procedure(newComm, numProcesses);

	MPI_Finalize();

	return 0;
}

void updateFixedPoints(ImageData data, Color (* image)[data.size], int start, int end);

void printImageLine(FILE* const out, const int len, const Color image[len])
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

void fillWithGray(const int size, Color arr[size])
{
	for (int i = 0; i < size; ++i)
		arr[i] = GRAY;
}

ImageData getImageData(const int myRank, MPI_Comm comm)
{
	ImageData imageData = { 0, 0, NULL };

	IF_COORDINATOR(myRank, {
		imageData = readImageFile("resources/img01.dat");
		printImageData(stdout, imageData);
	});

	MPI_Bcast(&imageData.size, 1, MPI_INT, COORDINATOR_RANK, comm);
	MPI_Bcast(&imageData.fixedPointCount, 1, MPI_INT, COORDINATOR_RANK, comm);

	imageData.fixedPoints = realloc(imageData.fixedPoints, imageData.fixedPointCount * sizeof(FixedPoint));
	MPI_Bcast(imageData.fixedPoints, imageData.fixedPointCount, FIXED_POINT_TYPE, COORDINATOR_RANK, comm);

	return imageData;
}

Color getPixelAt(
	int i, int j,
	const int height, const int width,
	const Color image[height][width],
	const Color top[width], const Color bottom[width])
{
	if (i < 0)
		return top[j];
	if (i >= height)
		return bottom[j];

	if (j < 0 || j >= width)
		return GRAY;

	return image[i][j];
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

void doStencilIteration(
	const int height, const int width,
	Color image[const height][width],
	const Color top[const width], const Color bottom[const width])
{
	const static int stencilOffsets[][2] = {
		{ 0, 0, }, { -1, 0, }, { +1, 0, }, { 0, -1, }, { 0, +1, },
	};

	Color newImage[height][width];

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			Color stencilPixels[5] = {};

			for (int p = 0; p < 5; ++p)
			{
				stencilPixels[p] = getPixelAt(
					i + stencilOffsets[p][0],
					j + stencilOffsets[p][1],
					height, width, image, top, bottom
				);
			}

			newImage[i][j] = averageColorOf(stencilPixels);
		}
	}

	memcpy(image, newImage, height * width * sizeof(Color));
}

void start_procedure(MPI_Comm comm, const int numProcesses)
{
	const int myRank = getProcessRank(comm);

	// TODO: Individualize fixed points
	const ImageData imageData = getImageData(myRank, comm);

	const int lineCount = imageData.size / numProcesses;
	const int start = lineCount * myRank; // Inclusive
	const int end = lineCount * (myRank + 1); // Exclusive

	debug_print(myRank, "Tasked with rows [%d, %d)\n", start, end);

	Color image[lineCount][imageData.size];

	for (int i = 0; i < lineCount; ++i)
		for (int j = 0; j < imageData.size; ++j)
			image[i][j] = WHITE;

	updateFixedPoints(imageData, image, start, end);

	Color fromTop[imageData.size];
	Color fromBottom[imageData.size];

	fillWithGray(imageData.size, fromTop);
	fillWithGray(imageData.size, fromBottom);

	const Neighbors myNeighbors = getNeighbors(comm);

	for (int i = 0; i < ITERATION_COUNT; ++i)
	{
		// Send down, receive up
		MPI_Sendrecv(
			&image[lineCount - 1][0], imageData.size, COLOR_TYPE, myNeighbors.down, TAG_ANY,
			&fromTop[0], imageData.size, COLOR_TYPE, myNeighbors.up, TAG_ANY, comm, MPI_STATUS_IGNORE
		);

		// Send up, receive down
		MPI_Sendrecv(
			&image[0][0], imageData.size, COLOR_TYPE, myNeighbors.up, TAG_ANY,
			&fromBottom[0], imageData.size, COLOR_TYPE, myNeighbors.down, TAG_ANY, comm, MPI_STATUS_IGNORE
		);

		doStencilIteration(lineCount, imageData.size, image, fromTop, fromBottom);

		updateFixedPoints(imageData, image, start, end);
	}

	Color(* finalImage)[imageData.size] = NULL;

	IF_COORDINATOR(myRank, {
		finalImage = malloc(imageData.size * sizeof *finalImage);
	});

	MPI_Gather(
		&image[0][0], imageData.size * lineCount, COLOR_TYPE,
		&finalImage[0][0], imageData.size * lineCount, COLOR_TYPE, COORDINATOR_RANK, comm
	);

	IF_COORDINATOR(myRank, {
		debug_print(myRank, "Output at \"output.txt\"\n");
		FILE* const f = fopen("output.txt", "w");
		printImage(f, imageData.size, imageData.size, finalImage);
		fclose(f);
	});

	free(imageData.fixedPoints);
	free(finalImage);
}

void updateFixedPoints(const ImageData data, Color (* image)[data.size], const int start, const int end)
{
	for (int i = 0; i < data.fixedPointCount; ++i)
	{
		const FixedPoint fp = data.fixedPoints[i];

		if (start <= fp.x && fp.x < end)
		{
			image[fp.x - start][fp.y] = fp.color;
		}
	}
}