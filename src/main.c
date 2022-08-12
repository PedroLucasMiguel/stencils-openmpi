#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ITERATION_COUNT 1000
#define N_CHANNELS 3
#define N_DIM 1
#define MAX_PROCESS 16
#define MANAGER_ID 0

#define FAKE_IMAGE_ROWS 256
#define FAKE_IMAGE_COLUMNS 256

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
    double*** matrixToProcess;
    double*** finalMatrix;

    int nProcesses; // Quantidade de processos que fazem parte do Cartesiano
    int processRank; // Rank do processo
    int dims; // Quantidade de dimensões para o novo Cartesiano
    int periods = 0; // Indica se o novo Cartesiano será periódico ou não (0 e N se conectam)
    int reorder = 1; // Indica se os processos no novo Cartesiano podem ser reordenados
    int coords; // Coordenada do processo dentro do Cartesiano
    int upProcess; // Processo superior
    int downProcess; // Processo inferior

    MPI_Init (&argc, &argv);

    // Retorna o número de processos (definido pelo parâmetro -np na execução)
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    // Define o valor de dimensão X do novo cartesiano baseado no número de processos
    dims = nProcesses;

    // Cria um novo cartesiano (newCom) com dimensão "N_DIM x dims" (1 x N)
    MPI_Cart_create(MPI_COMM_WORLD, N_DIM, &dims, &periods, reorder, &newComm);

    // Armazena o rank do processo em "processRank"
    MPI_Comm_rank(newComm, &processRank);

    // Recebe a localização desse processo dentro do Cartesiano e armazena em "coords"
    MPI_Cart_get(newComm, MAX_PROCESS, &dims, &periods, &coords);

    // Recebe a localização do processo superior (se existir) e inferior (se existir)
    MPI_Cart_shift(newComm, 0, 1, &upProcess, &downProcess);

    MPI_Finalize();
}