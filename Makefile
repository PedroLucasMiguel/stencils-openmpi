PROG := stencils-openmpi

SRCS := main.c $(wildcard src/*.c)
INCLUDE = -I.include/

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O3

EXEC := mpirun
MPI_FLAGS := -np 4 --oversubscribe --use-hwthread-cpus
PROG_FLAGS := resources/img01.dat output.txt

default: all

all: $(PROG) run

$(PROG): $(SRCS)
	$(CC) -o $@ $(SRCS) $(INCLUDE)

run:
	$(EXEC) $(MPI_FLAGS) $(PROG) $(PROG_FLAGS)