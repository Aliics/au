#include "std_inlined.h"

#include <stdio.h>

#include "util.h"

AuVar au_os_is_linux(AuVar *)
{
#ifdef __unix__
    return au_static_true;
#else
    return au_static_false;
#endif
}

AuVar au_os_is_macos(AuVar *)
{
#ifdef __APPLE__
    return au_static_true;
#else
    return au_static_false;
#endif
}

AuVar au_os_is_windows(AuVar *)
{
#ifdef __CYGWIN__
    return au_static_true;
#else
    return au_static_false;
#endif
}

AuVar au_io_puts(AuVar *in)
{
    ASSERT(in->type == AuString, "Input must be a string\n");
    printf("%s\n", in->data.string_val);
    return au_static_nil;
}

