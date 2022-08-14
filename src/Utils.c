#include <stdio.h>
#include <stdarg.h>

#include "../include/Utils.h"

int const COORDINATOR_RANK = 0;

/* A utility function to print a message to stdout with an added identification from the corresponding process */
__attribute__((format(printf, 2, 3)))
void ranked_print(int rank, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("%d: ", rank);
	vprintf(fmt, args);

	va_end(args);
}

void fillWithGray(const int len, Color arr[len])
{
	for (int i = 0; i < len; ++i)
	{
		arr[i] = GRAY;
	}
}