#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <mpi.h>

#include "Image.h"

int getNumProcesses(MPI_Comm comm);
int getProcessRank(MPI_Comm comm);

MPI_Comm arrangeProcesses(int numProcesses);

// This enum aids code readability when accessing struct fields indexed by what 'kind' of neighbor is being dealt with.
enum
{
	TOP = 0,
	BOTTOM = 1,
};

// This struct hosts a processes' neighbors' IDs
typedef struct
{
	int top;
	int bottom;
} NeighborsIds;

NeighborsIds getNeighborsIds(MPI_Comm comm);

ImageData getBroadcastImageData(int myRank, MPI_Comm comm, const char* filePath);

#endif // COMMUNICATION_H
