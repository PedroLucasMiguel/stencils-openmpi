# stencils-openmpi
A stencil program using Open MPI

# To compile:
```shell
mpicc -o out main.c && mpirun -np <n_processes> out
```