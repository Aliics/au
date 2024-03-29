#ifndef AU_UTIL_H
#define AU_UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond, fmt, ...)                                                                                         \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        fprintf(stderr, fmt "\n" __VA_OPT__(, ) __VA_ARGS__);                                                          \
        exit(1);                                                                                                       \
    }                                                                                                                  \
    else                                                                                                               \
        NULL

#define LENGTH(X) (sizeof X / sizeof X[0])

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct IntRange
{
    int start;
    int end;
} IntRange;

IntRange int_range(int, int);

#endif
