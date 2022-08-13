#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>

#include "../include/FileReader.h"

enum MessageTags
{
	TAG_ANY = 1,
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

#define LINEAR 0
#define ADJACENT 1

#define RANK_COORDINATOR 0

#define IF_COORDINATOR(RANK, CODE) \
    if ((RANK) == RANK_COORDINATOR) {\
        do {                  \
            CODE              \
        } while (0);          \
    }0

#define GRAY (Color){ 127, 127, 127, }

#define ITERATION_COUNT 10000

Neighbors getNeighbors(MPI_Comm comm)
{
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

void printImageLine(const int len, const Color image[len])
{
	for (int i = 0; i < len; ++i)
	{
		printColor(stdout, image[i]);
		putchar(' ');
	}
	putchar('\n');
}

void printImage(const int height, const int width, const Color image[const height][width])
{
	for (int i = 0; i < height; ++i)
	{
		printImageLine(width, image[i]);
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

	MPI_Bcast(&imageData.size, 1, MPI_INT, RANK_COORDINATOR, comm);
	MPI_Bcast(&imageData.fixedPointCount, 1, MPI_INT, RANK_COORDINATOR, comm);

	imageData.fixedPoints = realloc(imageData.fixedPoints, imageData.fixedPointCount * sizeof(FixedPoint));
	MPI_Bcast(imageData.fixedPoints, imageData.fixedPointCount, FIXED_POINT_TYPE, RANK_COORDINATOR, comm);

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
	Color result = { 0, 0, 0, };

	for (int i = 0; i < 5; ++i)
	{
		result.r += colors[i].r;
		result.g += colors[i].g;
		result.b += colors[i].b;
	}

	return (Color){ result.r / 5, result.g / 5, result.b / 5, };
}
void doStencilIteration(
	const int height, const int width,
	Color image[const height][width],
	const Color top[const width], const Color bottom[const width])
{
	Color newImage[height][width];
	memset(newImage, 0, sizeof newImage);

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			Color neighborUp = getPixelAt(i - 1, j, height, width, image, top, bottom);
			Color neighborDown = getPixelAt(i + 1, j, height, width, image, top, bottom);
			Color neighborLeft = getPixelAt(i, j - 1, height, width, image, top, bottom);
			Color neighborRight = getPixelAt(i, j + 1, height, width, image, top, bottom);

			newImage[i][j] = averageColorOf((Color[]){
				image[i][j], neighborUp, neighborDown, neighborLeft, neighborRight });
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
			image[i][j] = (Color){ 0, 0, 0 };

	updateFixedPoints(imageData, image, start, end);

//	debug_print(myRank, "My image:\n");
//	printImage(lineCount, imageData.size, image);

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
		&finalImage[0][0], imageData.size * lineCount, COLOR_TYPE, RANK_COORDINATOR, comm
	);

	MPI_Barrier(comm);
	IF_COORDINATOR(myRank, {
		debug_print(myRank, "Final Image:\n");
		printImage(imageData.size, imageData.size, finalImage);
	});

//	debug_print(myRank, "up: %d | down: %d\n", myNeighbors.up, myNeighbors.down);





//	updateFixedPoints(imageData, image, start, end);



	free(imageData.fixedPoints);
}

void updateFixedPoints(const ImageData data, Color (* image)[data.size], const int start, const int end)
{
	for (int i = 0; i < data.fixedPointCount; ++i)
	{
		const FixedPoint fp = data.fixedPoints[i];

		if (start <= fp.y && fp.y < end)
		{
			image[fp.x - start][fp.y] = fp.color;
		}
	}
}