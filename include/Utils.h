#ifndef UTILS_H
#define UTILS_H

#include "Color.h"

extern int const COORDINATOR_RANK;

#define IF_COORDINATOR(RANK, CODE) \
    if ((RANK) == COORDINATOR_RANK) {\
        do {                  \
            CODE              \
        } while (0);          \
    }0

enum MessageTags
{
	TAG_SEND_UP = 1,
	TAG_SEND_DOWN,
};

void debug_print(int rank, const char* fmt, ...);

void fillWithGray(int len, Color arr[len]);

#endif // UTILS_H
