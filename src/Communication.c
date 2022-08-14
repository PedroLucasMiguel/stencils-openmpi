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

/* This function arranges the processes in a straight, non-periodic line. */
MPI_Comm arrangeProcesses(int numProcesses)
{
	MPI_Comm newComm;
	MPI_Cart_create(MPI_COMM_WORLD, 1, (int[]){ numProcesses }, (int[]){ false }, true, &newComm);
	return newComm;
}

/* This function fetches the process ids of the adjacent processes from this one.
 * If the process in on a boundary, MPI_PROC_NULL is returned by MPI_Cart_shift on the respective field.
 * Sending or receiving to/from MPI_PROC_NULL behaves as expected, it does nothing.
 */
NeighborsIds getNeighborsIds(MPI_Comm comm)
{
#define LINEAR 0
#define ADJACENT 1
	int up;
	int down;
	MPI_Cart_shift(comm, LINEAR, ADJACENT, &up, &down);
	return (NeighborsIds){ up, down };
}

/* This function is responsible for getting the image data (size, fixed points) from the file in 'filePath' and
 * distributing this information among all processes through a broadcast.
 * It is important to note that the structure imageData cannot be made into a 'mpi struct' since the field
 * 'fixedPoints' is variably sized. Thus, the fields must be transmitted one by one.
 */
ImageData getBroadcastImageData(int myRank, MPI_Comm comm, const char* filePath)
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