#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define ITERATION_COUNT 1000
#define N_CHANNELS 3

void Init_matrix(double*** matrix, int rows, int columns) {

    int i = 0;
    int j = 0;

    matrix = (double***)malloc(rows * sizeof(double**));
    
    for(i = 0; i < rows; i++) {
        matrix[i] = (double**)malloc(columns * sizeof(double*));
        
        for(j = 0; j < columns; j++) {
            matrix[i][j] = (double*)malloc(N_CHANNELS * sizeof(double)); 
        }
    }

}


int main(int argc, char** argv) {

    MPI_Comm newComm; // Recebera o novo "universo"
    
    // Matrizes
    double*** red;
    double*** black;
    double*** final;

    int nProcesses; // Recebe a quantidade de processos que fazem parte do Comm
    int processID; // Rebe o ID do processo

    MPI_Init (&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    printf("%d\n", nProcesses);

    MPI_Finalize();
}