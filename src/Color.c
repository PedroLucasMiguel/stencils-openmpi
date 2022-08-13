#include "../include/Color.h"

Color readColorFromFile(FILE* f)
{
	Color c;
	fscanf(f, "%d %d %d", &c.channels[R], &c.channels[G], &c.channels[B]);
	return c;
}

void printColor(FILE* out, const Color c)
{
	fprintf(out, "< %d, %d, %d >", c.channels[R], c.channels[G], c.channels[B]);
}

MPI_Datatype getColorDatatype()
{
	static MPI_Datatype committedDatatype = MPI_DATATYPE_NULL;

	if (committedDatatype != MPI_DATATYPE_NULL)
		return committedDatatype;

	MPI_Type_create_struct(
		1, (int[]){ 3 },
		(MPI_Aint[]){ offsetof(Color, channels), },
		(MPI_Datatype[]){ MPI_INT },
		&committedDatatype
	);

	MPI_Type_commit(&committedDatatype);

	return committedDatatype;
}