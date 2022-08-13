PROG := stencils-openmpi

SRCS := main.c $(wildcard src/*.c)
INCLUDE = -I.include/

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic

EXEC := mpirun
EXECFLAGS := -np 4

default: all

all: $(PROG) run

$(PROG): $(SRCS)
	$(CC) -o $@ $(SRCS) $(INCLUDE)

run:
	$(EXEC) $(EXECFLAGS) $(PROG)