#ifndef AU_PARSE_H
#define AU_PARSE_H

#include "runtime.h"
#include "token.h"
#include "util.h"

AuVar parse(AuRuntime *, const AuToken *, int, int);
AuVar parse_expr(AuRuntime *, const AuToken *, int, IntRange);

#endif
