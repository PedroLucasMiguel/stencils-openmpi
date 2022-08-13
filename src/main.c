#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <mpi.h>

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

void printImage(const int iSize, const int jSize, const Color image[const iSize][jSize])
{
	for (int i = 0; i < iSize; ++i)
	{
		printImageLine(jSize, image[i]);
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
		imageData = readImageFile("resources/small_test.dat");
		printImageData(stdout, imageData);
	});

	MPI_Bcast(&imageData.size, 1, MPI_INT, RANK_COORDINATOR, comm);
	MPI_Bcast(&imageData.fixedPointCount, 1, MPI_INT, RANK_COORDINATOR, comm);

	imageData.fixedPoints = realloc(imageData.fixedPoints, imageData.fixedPointCount * sizeof(FixedPoint));
	MPI_Bcast(imageData.fixedPoints, imageData.fixedPointCount, FIXED_POINT_TYPE, RANK_COORDINATOR, comm);

	return imageData;
}

void start_procedure(MPI_Comm comm, const int numProcesses)
{
	const int myRank = getProcessRank(comm);

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

	debug_print(myRank, "My image:\n");
	printImage(lineCount, imageData.size, image);

	Color fromTop[imageData.size];
	fillWithGray(imageData.size, fromTop);

	Color fromBottom[imageData.size];
	fillWithGray(imageData.size, fromBottom);

	const Neighbors myNeighbors = getNeighbors(comm);
	debug_print(myRank, "up: %d | down: %d\n", myNeighbors.up, myNeighbors.down);

	// Send down, receive up
	MPI_Sendrecv(
		&image[lineCount - 1][0], imageData.size, COLOR_TYPE, myNeighbors.down, TAG_ANY,
		&fromTop[0], imageData.size, COLOR_TYPE, myNeighbors.up, TAG_ANY, comm, MPI_STATUS_IGNORE
	);

	debug_print(myRank, "receive from [up] %d:", myNeighbors.up);
	printImageLine(imageData.size, fromTop);

	// Send up, receive down
	MPI_Sendrecv(
		&image[0][0], imageData.size, COLOR_TYPE, myNeighbors.up, TAG_ANY,
		&fromBottom[0], imageData.size, COLOR_TYPE, myNeighbors.down, TAG_ANY, comm, MPI_STATUS_IGNORE
	);

	debug_print(myRank, "receive from [down] %d:", myNeighbors.down);
	printImageLine(imageData.size, fromBottom);

//	updateFixedPoints(imageData, image, start, end);

//	debug_print(myRank, "My image:\n");
//	printImage(lineCount, imageData.size, image);

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