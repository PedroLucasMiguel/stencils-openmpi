#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "../include/FileReader.h"

#define MAX_PROCESS 16
#define N_DIM 1

#define NON_PERIODIC &(int){0}
#define REARRANGE (int){1}

int main(int argc, char** argv)
{
	MPI_Comm newComm;

	int numProcesses;

	char test_0[64] = "Hello World!\0";
	char test_1[64] = "World Hello!\0";
	char test2[64];
	double test_00[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
	double test2_2[5];

	MPI_Init(&argc, &argv);

	// Retorna o número de processos (definido pelo parâmetro -np na execução)
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	// Cria um novo cartesiano (newComm) com dimensão "N_DIM x dims" (1 x N)
	MPI_Cart_create(MPI_COMM_WORLD, N_DIM, &numProcesses, NON_PERIODIC, REARRANGE, &newComm);

	int processRank;
	MPI_Comm_rank(newComm, &processRank);

	printf("Hello from process %d/%d\n", processRank, numProcesses);

	// Recebe a localização desse processo dentro do Comunicador e armazena em "coords"
	int coords; // Coordenada do processo dentro do Comunicador
	MPI_Cart_get(newComm, MAX_PROCESS, &numProcesses, NON_PERIODIC, &coords);

	// Recebe a localização do processo superior (se existir) e inferior (se existir)
	int upProcess; // Processo superior
	int downProcess; // Processo inferior
	MPI_Cart_shift(newComm, 0, 1, &upProcess, &downProcess);

	MPI_Datatype mpi_FixedPoint = addFixedPointToMpi();

	// DEBUG AREA---------------------------------------------------------------------------
	// Exemplo de send and recieve funcional!
	if (processRank == 0)
	{
		ImageData d = readImageFile("resources/img01.dat");
		printImageData(&d);

		MPI_Send(test_0, 64, MPI_CHAR, downProcess, 69420, newComm);
		MPI_Recv(test2, 64, MPI_CHAR, downProcess, 69420, newComm, MPI_STATUS_IGNORE);
		MPI_Send(test_00, 5, MPI_DOUBLE, downProcess, 42069, newComm);

		MPI_Send(&(FixedPoint){ 6, 9, 4, 2, 0 }, 1, mpi_FixedPoint, downProcess, 1212, newComm);
		printf("Process 0 received: %s\n", test2);
	}

	else if (processRank == 1)
	{
		MPI_Send(test_1, 64, MPI_CHAR, upProcess, 69420, newComm);
		MPI_Recv(test2, 64, MPI_CHAR, upProcess, 69420, newComm, MPI_STATUS_IGNORE);

		printf("Process 1 received: %s\n", test2);

		MPI_Recv(test2_2, 5, MPI_DOUBLE, upProcess, 42069, newComm, MPI_STATUS_IGNORE);

		for (int i = 0; i < 5; i++)
		{
			printf("%f ", test2_2[i]);
		}
		printf("\n");

		FixedPoint r;
		MPI_Recv(&r, 1, mpi_FixedPoint, upProcess, 1212, newComm, MPI_STATUS_IGNORE);

		printFixedPoint(&r);
	}

	MPI_Finalize();

	return 0;
}