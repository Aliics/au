#ifndef AU_PARSE_H
#define AU_PARSE_H

#include "token.h"
#include "runtime.h"
#include "util.h"

void parse(const AuRuntime *, const AuToken *, int);
AuVar parse_expr(const AuRuntime *, const AuToken *, int, IntRange);

#endif
