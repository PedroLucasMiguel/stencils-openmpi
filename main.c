#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "include/Communication.h"
#include "include/Utils.h"
#include "include/Iteration.h"

void runProcedure(MPI_Comm comm, int numProcesses, const char* inFilePath, const char* outFilePath);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	const int numProcesses = getNumProcesses(MPI_COMM_WORLD);
	MPI_Comm newComm = arrangeProcesses(numProcesses);

	runProcedure(newComm, numProcesses, "resources/img01.dat", "output.txt");

	MPI_Finalize();

	return 0;
}

void runProcedure(MPI_Comm comm, int numProcesses, const char* inFilePath, const char* outFilePath)
{
	const int myRank = getProcessRank(comm);

	ImageData imageData = getBroadcastImageData(myRank, comm, inFilePath);

	const int lineCount = imageData.size / numProcesses;
	const int start = lineCount * myRank; // Inclusive
	const int end = lineCount * (myRank + 1); // Exclusive

	debug_print(myRank, "Tasked with rows [%d, %d)\n", start, end);

	filterFixedPoints(&imageData, start, end);

	Color myImage[lineCount][imageData.size];

	for (int i = 0; i < lineCount; ++i)
		for (int j = 0; j < imageData.size; ++j)
			myImage[i][j] = WHITE;

	setFixedPointsOnImageSlice(imageData, myImage);

	Color adjLines[2][imageData.size];
	fillWithGray(imageData.size, adjLines[0]);
	fillWithGray(imageData.size, adjLines[1]);

	const NeighborsIds neighborsIds = getNeighborsIds(comm);

	for (int i = 0; i < ITERATION_COUNT; ++i)
	{
		IF_COORDINATOR(myRank, {
			if (i % 1000 == 0)
			{
				debug_print(myRank, "Starting iteration #%d\n", i);
			}
		});

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

		setFixedPointsOnImageSlice(imageData, myImage);
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
		debug_print(myRank, "Result image output to %s\n", outFilePath);
		FILE* const f = fopen(outFilePath, "w");
		printImage(f, imageData.size, imageData.size, finalImage);
		fclose(f);
	});

	free(imageData.fixedPoints);
	free(finalImage);
}


