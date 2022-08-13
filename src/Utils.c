#include <stdio.h>
#include <stdarg.h>

#include "../include/Utils.h"

int const COORDINATOR_RANK = 0;

__attribute__((format(printf, 2, 3)))
void debug_print(const int rank, const char* const fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("%d: ", rank);
	vprintf(fmt, args);

	va_end(args);
}