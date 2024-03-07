#ifndef AU_UTIL_H
#define AU_UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond, fmt, ...)                                                                                         \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        fprintf(stderr, fmt __VA_OPT__(, ) __VA_ARGS__);                                                               \
        exit(1);                                                                                                       \
    }

#define LENGTH(X) (sizeof X / sizeof X[0])

#endif
