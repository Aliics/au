#ifndef AU_PARSE_H
#define AU_PARSE_H

#include "runtime.h"
#include "token.h"
#include "util.h"

typedef struct AuParseCtx
{
    AuRuntime *rt;
    AuToken *tks;
    int tks_len;
    int ln;
} AuParseCtx;

AuVar parse(AuParseCtx *, IntRange, int);
AuVar parse_expr(AuParseCtx *, IntRange);

#endif
