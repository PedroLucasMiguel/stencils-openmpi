#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include <mpi.h>

enum Channel
{
	R = 0,
	G = 1,
	B = 2,
};

typedef struct
{
	int channels[3];
} Color;

#define GRAY (Color){ 127, 127, 127, }
#define WHITE (Color){ 0, 0, 0, }

MPI_Datatype getColorDatatype();
#define COLOR_TYPE getColorDatatype()

Color readColorFromFile(FILE* f);
void printColor(FILE* out, Color c);

#endif // COLOR_H
