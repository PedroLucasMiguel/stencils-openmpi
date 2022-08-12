# stencils-openmpi
A stencil program using Open MPI

# To compile:
```shell
mpicc -o out -std=c99 -pedantic -Wall -Werror main.c && mpirun -np <N_PROCESSES> out
```