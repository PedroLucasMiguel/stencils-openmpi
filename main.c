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
	const MPI_Comm newComm = arrangeProcesses(numProcesses);

	const char* inFile = "";
	const char* outFile = "";

	const int myRank = getProcessRank(newComm);

	// Single command-line argument check, just checks if 2 arguments were passed in.
	IF_COORDINATOR(myRank, {
		if ((argc--) < 2) {
			fprintf(stderr, "Usage: %s inputFile outputFile\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = argv[1];
		outFile = argv[2];
	});

	runProcedure(newComm, numProcesses, inFile, outFile);

	MPI_Finalize();

	return 0;
}

/* The core function of the program. Coordinator reads file from 'inFilePath', distributes
 * the image data to the other processes, which all iterate over the image to apply the stencil,
 * and collects the resulting image, outputting it to 'outFilePath'.
 */
void runProcedure(MPI_Comm comm, int numProcesses, const char* inFilePath, const char* outFilePath)
{
	const int myRank = getProcessRank(comm);

	ImageData imageData = getBroadcastImageData(myRank, comm, inFilePath);

	// Timer starts after file is read, and processes are ready to begin the process
	const double startTime = MPI_Wtime();

	const int lineCount = imageData.size / numProcesses;
	const int start = lineCount * myRank; // Inclusive
	const int end = lineCount * (myRank + 1); // Exclusive

	ranked_print(myRank, "Tasked with rows [%d, %d)\n", start, end);

	transformFixedPoints(&imageData, start, end);

	// This matrix hosts the section of the image this process is responsible for.
	Color myImage[lineCount][imageData.size];

	for (int i = 0; i < lineCount; ++i)
		for (int j = 0; j < imageData.size; ++j)
			myImage[i][j] = WHITE;

	setFixedPointsOnImageSlice(imageData, myImage);

	// This matrix hosts the lines received from neighboring processes.
	Color adjLines[2][imageData.size];
	fillWithGray(imageData.size, adjLines[0]);
	fillWithGray(imageData.size, adjLines[1]);

	const NeighborsIds neighborsIds = getNeighborsIds(comm);

	for (int i = 0; i < ITERATION_COUNT; ++i)
	{
		IF_COORDINATOR(myRank, {
			if (i % 1000 == 0)
			{
				ranked_print(myRank, "Starting iteration #%d\n", i);
			}
		});

		// Send to neighbor on top, receive from neighbor on bottom
		MPI_Sendrecv(
			&myImage[0][0], imageData.size, COLOR_TYPE, neighborsIds.top, TAG_SEND_TOP,
			&adjLines[BOTTOM], imageData.size, COLOR_TYPE, neighborsIds.bottom, TAG_SEND_TOP, comm, MPI_STATUS_IGNORE
		);

		// And vice-versa...
		MPI_Sendrecv(
			&myImage[lineCount - 1][0], imageData.size, COLOR_TYPE, neighborsIds.bottom, TAG_SEND_BOTTOM,
			&adjLines[TOP], imageData.size, COLOR_TYPE, neighborsIds.top, TAG_SEND_BOTTOM, comm, MPI_STATUS_IGNORE
		);

		/* NOTE: Processes on the 'boundary' or the vector created by MPI will communicate with MPI_PROC_NULL.
		 * This behaves as expected, with buffers left unmodified. Thus, processes without neighbors will have an array
		 * from adjLines still filled with gray, mimic-ing the imaginary gray border around the image.
		 */

		// Apply the stencil over our slice of the image
		doStencilIteration((IterationData){
			lineCount, imageData.size,
			(Color*)myImage,
			{ adjLines[0], adjLines[1] }
		});

		// After every iteration, update the fixed points on the image
		setFixedPointsOnImageSlice(imageData, myImage);
	}

	// Timer ends after final stencil iteration is applied.
	const double endTime = MPI_Wtime();

	// Resulting image. Subordinated processes will not use this variable, so memory will not be allocated for them.
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
		ranked_print(myRank, "Process took %f seconds.\n", endTime - startTime);
		ranked_print(myRank, "Result image output to %s\n", outFilePath);
		FILE* const f = fopen(outFilePath, "w");

		if (f == NULL)
		{
			perror("Unable to open file");
			exit(EXIT_FAILURE);
		}

		printImage(f, imageData.size, imageData.size, finalImage);
		fclose(f);
	});

	free(imageData.fixedPoints);
	// free(NULL) is guaranteed to do nothing, so no UB on subordinated processes.
	free(finalImage);
}


