#include <stdio.h>

#include "util.h"

char *read_file(const char *filename, int *out_len)
{
    FILE *f = fopen(filename, "r");
    ASSERT(f, "File %s could not be opened", filename);

    fseek(f, 0, SEEK_END);
    *out_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *src = malloc(*out_len + 1);
    ASSERT(src, "Could not allocate space for file %s", filename);

    fread(src, 1, *out_len, f);
    src[*out_len] = '\0';

    fclose(f);

    return src;
}
