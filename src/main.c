#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <mpi.h>

#define MAX_PROCESS 16

#define NON_PERIODIC &(int){0}

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

int getNumProcesses()
{
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
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
#define DISPLACEMENT 1

Neighbors getNeighbors(MPI_Comm comm)
{
	int up;
	int down;
	MPI_Cart_shift(comm, LINEAR, DISPLACEMENT, &up, &down);
	return (Neighbors){ up, down };
}

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	const int numProcesses = getNumProcesses();

	MPI_Comm newComm = arrangeProcesses(numProcesses);

	const int myRank = getProcessRank(newComm);
	debug_print(myRank, "hello!\n");

	const Neighbors myNeighbors = getNeighbors(newComm);

	debug_print(myRank, "up: %d | down: %d\n", myNeighbors.up, myNeighbors.down);

	MPI_Finalize();

	return 0;
}