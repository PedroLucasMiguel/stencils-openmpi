PROG := stencils-openmpi

SRCS := $(wildcard src/*.c)
INCLUDE = -I.include/

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic

OBJS := $(SRCS:.c=.o)

default: all

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(INCLUDE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	rm -f $(OBJS)