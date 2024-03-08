#ifndef AU_STD_INLINED_H
#define AU_STD_INLINED_H
#include "runtime.h"

AuVar au_os_is_linux(AuVar *);

AuVar au_os_is_macos(AuVar *);

AuVar au_os_is_windows(AuVar *);

AuVar au_io_puts(AuVar *);

static const AuFunction au_os_is_linux_function = {
        .name = "is_linux?",
        .name_len = 9,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_linux,
        .params_len = 0,
};

static const AuFunction au_os_is_macos_function = {
        .name = "is_macos?",
        .name_len = 9,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_macos,
        .params_len = 0,
};

static const AuFunction au_os_is_windows_function = {
        .name = "is_windows?",
        .name_len = 11,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_windows,
        .params_len = 0,
};

static const AuFunction au_io_puts_function = {
        .name = "puts",
        .name_len = 4,
        .type = AuFnRef,
        .fn.fn_ref = au_io_puts,
        .params_len = 1,
};

#endif
