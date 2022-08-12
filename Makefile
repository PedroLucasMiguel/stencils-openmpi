PROG := stencils-openmpi

SRCS := $(wildcard src/*.c)
INCLUDE = -I.include/

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic

EXEC := mpirun
EXECFLAGS := -np 4

default: all

all: $(PROG) clean run

$(PROG): $(SRCS)
	$(CC) -o $@ $(SRCS) $(INCLUDE)

clean:
	rm -f $(OBJS)

run:
	$(EXEC) $(EXECFLAGS) $(PROG)