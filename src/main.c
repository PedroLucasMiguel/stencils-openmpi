#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "../include/Color.h"
#include "../include/FileReader.h"

enum MessageTags
{
	TAG_SEND_UP = 1,
	TAG_SEND_DOWN,
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
} NeighborsIds;

#define COORDINATOR_RANK 0

#define IF_COORDINATOR(RANK, CODE) \
    if ((RANK) == COORDINATOR_RANK) {\
        do {                  \
            CODE              \
        } while (0);          \
    }0



#define ITERATION_COUNT 10000

NeighborsIds getNeighborsIds(MPI_Comm comm)
{
#define LINEAR 0
#define ADJACENT 1
	int up;
	int down;
	MPI_Cart_shift(comm, LINEAR, ADJACENT, &up, &down);
	return (NeighborsIds){ up, down };
}

void start_procedure(MPI_Comm comm, int numProcesses, const char* inFilePath, const char* outFilePath);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	const int numProcesses = getNumProcesses(MPI_COMM_WORLD);
	MPI_Comm newComm = arrangeProcesses(numProcesses);

	start_procedure(newComm, numProcesses, "resources/img01.dat", "output.txt");

	MPI_Finalize();

	return 0;
}

void setFixedPointsOnImage(ImageData data, Color (* image)[data.size]);

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

enum
{
	TOP = 0,
	BOTTOM = 1,
};

void fillWithGray(const int size, Color arr[2][size])
{
	for (int i = 0; i < size; ++i)
	{
		arr[TOP][i] = GRAY;
		arr[BOTTOM][i] = GRAY;
	}
}

ImageData getImageData(const int myRank, MPI_Comm comm, const char* const filePath)
{
	ImageData imageData = { 0, 0, NULL };

	IF_COORDINATOR(myRank, {
		imageData = readImageDataFile(filePath);
		printImageData(stdout, imageData);
	});

	MPI_Bcast(&imageData.size, 1, MPI_INT, COORDINATOR_RANK, comm);
	MPI_Bcast(&imageData.fixedPointCount, 1, MPI_INT, COORDINATOR_RANK, comm);

	imageData.fixedPoints = realloc(imageData.fixedPoints, imageData.fixedPointCount * sizeof(FixedPoint));
	MPI_Bcast(imageData.fixedPoints, imageData.fixedPointCount, FIXED_POINT_TYPE, COORDINATOR_RANK, comm);

	return imageData;
}

typedef struct
{
	int h;
	int w;
	Color* image;
	Color* adj[2];
} IterationData;

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

FixedPoint* filterFixedPoints(ImageData* const imageData, const int start, const int end)
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

	return filtered;
}

void start_procedure(MPI_Comm comm, const int numProcesses, const char* const inFilePath, const char* const outFilePath)
{
	const int myRank = getProcessRank(comm);

	ImageData imageData = getImageData(myRank, comm, inFilePath);

	const int lineCount = imageData.size / numProcesses;
	const int start = lineCount * myRank; // Inclusive
	const int end = lineCount * (myRank + 1); // Exclusive

	debug_print(myRank, "Tasked with rows [%d, %d)\n", start, end);

	filterFixedPoints(&imageData, start, end);

	Color myImage[lineCount][imageData.size];

	for (int i = 0; i < lineCount; ++i)
		for (int j = 0; j < imageData.size; ++j)
			myImage[i][j] = WHITE;

	setFixedPointsOnImage(imageData, myImage);

	Color adjLines[2][imageData.size];
	fillWithGray(imageData.size, adjLines);

	const NeighborsIds neighborsIds = getNeighborsIds(comm);

	for (int i = 0; i < ITERATION_COUNT; ++i)
	{
		// Send to neighbor up, receive from neighbor down
		MPI_Sendrecv(
			&myImage[0][0], imageData.size, COLOR_TYPE, neighborsIds.up, TAG_SEND_UP,
			&adjLines[BOTTOM], imageData.size, COLOR_TYPE, neighborsIds.down, TAG_SEND_UP, comm, MPI_STATUS_IGNORE
		);

		// And vice-versa...
		MPI_Sendrecv(
			&myImage[lineCount - 1][0], imageData.size, COLOR_TYPE, neighborsIds.down, TAG_SEND_DOWN,
			&adjLines[TOP], imageData.size, COLOR_TYPE, neighborsIds.up, TAG_SEND_DOWN, comm, MPI_STATUS_IGNORE
		);

		doStencilIteration((IterationData){
			lineCount, imageData.size,
			(Color*)myImage,
			{ adjLines[0], adjLines[1] }
		});

		setFixedPointsOnImage(imageData, myImage);
	}

	Color(* finalImage)[imageData.size] = NULL;

	IF_COORDINATOR(myRank, {
		finalImage = malloc(imageData.size * sizeof *finalImage);
	});

	MPI_Gather(
		&myImage[0][0], imageData.size * lineCount, COLOR_TYPE,
		&finalImage[0][0], imageData.size * lineCount, COLOR_TYPE,
		COORDINATOR_RANK, comm
	);

	IF_COORDINATOR(myRank, {
		debug_print(myRank, "Output at %s\n", outFilePath);
		FILE* const f = fopen(outFilePath, "w");
		printImage(f, imageData.size, imageData.size, finalImage);
		fclose(f);
	});

	free(imageData.fixedPoints);
	free(finalImage);
}

void setFixedPointsOnImage(const ImageData data, Color (* image)[data.size])
{
	for (int i = 0; i < data.fixedPointCount; ++i)
	{
		const FixedPoint fp = data.fixedPoints[i];
		image[fp.x][fp.y] = fp.color;
	}
}