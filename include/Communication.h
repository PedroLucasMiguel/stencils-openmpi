#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <mpi.h>

#include "Image.h"

int getNumProcesses(MPI_Comm comm);
int getProcessRank(MPI_Comm comm);

MPI_Comm arrangeProcesses(int numProcesses);

enum
{
	TOP = 0,
	BOTTOM = 1,
};

typedef struct
{
	int up;
	int down;
} NeighborsIds;

NeighborsIds getNeighborsIds(MPI_Comm comm);

ImageData getBroadcastImageData(int myRank, MPI_Comm comm, const char* filePath);

#endif // COMMUNICATION_H
