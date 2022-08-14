#ifndef UTILS_H
#define UTILS_H

#include "Color.h"

extern int const COORDINATOR_RANK;

/* Convenience macro that executes code if and only if the currently executing processes is deemed 'the coordinator',
 * i.e., its rank equals the rank contained in the variable COORDINATOR_RANK.
 */
#define IF_COORDINATOR(RANK, CODE)        \
    do {                                  \
        if ((RANK) == COORDINATOR_RANK) { \
            CODE                          \
        }                                 \
    } while (0)

/* Message tags to differentiate sets of sends/receives between each other.
 * Functionally, they are not used, since in this application all communications happen 'at the same stage' of each process.
 * However, they are still beneficial since they aid code readability.
 */
enum MessageTags
{
	TAG_SEND_TOP = 1,
	TAG_SEND_BOTTOM,
};

void ranked_print(int rank, const char* fmt, ...);

void fillWithGray(int len, Color arr[len]);

#endif // UTILS_H
