#ifndef AU_ASSERT_H
#define AU_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond, fmt, ...)                                                                                         \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        fprintf(stderr, fmt __VA_OPT__(, ) __VA_ARGS__);                                                               \
        exit(1);                                                                                                       \
    }

#endif
