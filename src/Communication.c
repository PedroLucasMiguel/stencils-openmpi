#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#include "../include/Communication.h"
#include "../include/Utils.h"

int getNumProcesses(MPI_Comm comm)
{
	int numProcesses;
	MPI_Comm_size(comm, &numProcesses);
	return numProcesses;
}

int getProcessRank(MPI_Comm comm)
{
	int myRank;
	MPI_Comm_rank(comm, &myRank);
	return myRank;
}

MPI_Comm arrangeProcesses(int numProcesses)
{
	MPI_Comm newComm;
	MPI_Cart_create(MPI_COMM_WORLD, 1, (int[]){ numProcesses }, (int[]){ false }, true, &newComm);
	return newComm;
}

NeighborsIds getNeighborsIds(MPI_Comm comm)
{
#define LINEAR 0
#define ADJACENT 1
	int up;
	int down;
	MPI_Cart_shift(comm, LINEAR, ADJACENT, &up, &down);
	return (NeighborsIds){ up, down };
}

ImageData getImageData(const int myRank, MPI_Comm comm, const char* const filePath)
{
	ImageData imageData = { 0, 0, NULL };

	IF_COORDINATOR(myRank, {
		imageData = readImageData(filePath);
		printImageData(stdout, imageData);
	});

	MPI_Bcast(&imageData.size, 1, MPI_INT, COORDINATOR_RANK, comm);
	MPI_Bcast(&imageData.fixedPointCount, 1, MPI_INT, COORDINATOR_RANK, comm);

	imageData.fixedPoints = realloc(imageData.fixedPoints, imageData.fixedPointCount * sizeof(FixedPoint));
	MPI_Bcast(imageData.fixedPoints, imageData.fixedPointCount, FIXED_POINT_TYPE, COORDINATOR_RANK, comm);

	return imageData;
}