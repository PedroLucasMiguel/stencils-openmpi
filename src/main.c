#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include <string.h>
#include "../include/FileReader.h"

#define ITERATION_COUNT 1000
#define MANAGER_RANK 0 // Rank do processo responsável pela leitura do arquivo e junção de matrizes
#define MAX_PROCESS 16
#define N_DIM 1
#define MSG_CODE 1212

int main(int argc, char** argv) {

    MPI_Comm newComm; // Novo Comunicador
    MPI_Status status; // Status da mensagem recebida

    int nProcesses; // Quantidade de processos que fazem parte do Comunicador
    int processRank; // Rank do processo
    int dims; // Quantidade de dimensões para o novo Comunicador
    int periods = 0; // Indica se o novo Comunicador será periódico ou não (0 e N se conectam)
    int reorder = 1; // Indica se os processos no novo Comunicador podem ser reordenados
    int coords; // Coordenada do processo dentro do Comunicador
    int upProcess; // Processo superior
    int downProcess; // Processo inferior

    // DEBUG
    char test_0[64] = "Hello World!\0";
    char test_1[64] = "World Hello!\0";
    char test2[64];
    double test_00[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double test2_2[5];

    FileData a;
    readFile("img01.dat", &a);
    printFileData(&a);

    MPI_Init (&argc, &argv);

    // Retorna o número de processos (definido pelo parâmetro -np na execução)
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    // Define o valor de dimensão X do novo cartesiano baseado no número de processos
    dims = nProcesses;

    // Cria um novo cartesiano (newCom) com dimensão "N_DIM x dims" (1 x N)
    MPI_Cart_create(MPI_COMM_WORLD, N_DIM, &dims, &periods, reorder, &newComm);

    // Armazena o rank do processo em "processRank"
    MPI_Comm_rank(newComm, &processRank);

    // Recebe a localização desse processo dentro do Comunicador e armazena em "coords"
    MPI_Cart_get(newComm, MAX_PROCESS, &dims, &periods, &coords);

    // Recebe a localização do processo superior (se existir) e inferior (se existir)
    MPI_Cart_shift(newComm, 0, 1, &upProcess, &downProcess);


    // DEBUG AREA---------------------------------------------------------------------------
    // Exemplo de send and recieve funcional!
    if (processRank == 0){
        MPI_Send(test_0, strlen(test_0), MPI_CHAR, downProcess, 69420, newComm);
        MPI_Recv(test2, strlen(test_1), MPI_CHAR, downProcess, 69420, newComm, &status);
        MPI_Send(test_00, 5, MPI_DOUBLE, downProcess, 42069, newComm);
        printf("Process 0 recieved: %s\n", test2);
    }

    else if (processRank == 1) {
        MPI_Send(test_1, strlen(test_1), MPI_CHAR, upProcess, 69420, newComm);
        MPI_Recv(test2, strlen(test_0), MPI_CHAR, upProcess, 69420, newComm, &status); 

        printf("Process 1 recieved: %s\n", test2);

        MPI_Recv(test2_2, 5, MPI_DOUBLE, upProcess, 42069, newComm, &status);

        for(int i = 0; i < 5; i ++) {
            printf("%f ", test2_2[i]);
        }
        printf("\n");
    }
    

    MPI_Finalize();

    return 0;
}