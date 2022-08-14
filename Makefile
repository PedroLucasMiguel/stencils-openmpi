PROG := stencils-openmpi

SRCS := main.c $(wildcard src/*.c)
INCLUDE = -I.include/

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O3

EXEC := mpirun
NP := 4
MPI_FLAGS := -np $(NP) --oversubscribe --use-hwthread-cpus
IN_FILE := img01.dat
OUT_FILE := output.txt
PROG_FLAGS := $(IN_FILE) $(OUT_FILE)

default: all

all: $(PROG)

$(PROG): $(SRCS)
	$(CC) -o $@ $(SRCS) $(INCLUDE)

run:
	$(EXEC) $(MPI_FLAGS) $(PROG) $(PROG_FLAGS)