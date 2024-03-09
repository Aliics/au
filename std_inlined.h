#ifndef AU_STD_INLINED_H
#define AU_STD_INLINED_H
#include "runtime.h"

AuVar au_os_is_linux(AuVar *);

AuVar au_os_is_macos(AuVar *);

AuVar au_os_is_windows(AuVar *);

AuVar au_io_puts(AuVar *);

static const AuFunction AU_OS_IS_LINUX_FN = {
        .name = "is_linux?",
        .name_len = 9,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_linux,
        .params_len = 0,
};

static const AuFunction AU_OS_IS_MACOS_FN = {
        .name = "is_macos?",
        .name_len = 9,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_macos,
        .params_len = 0,
};

static const AuFunction AU_OS_IS_WINDOWS_FN = {
        .name = "is_windows?",
        .name_len = 11,
        .type = AuFnRef,
        .fn.fn_ref = au_os_is_windows,
        .params_len = 0,
};

static const AuFunction AU_IO_PUTS_FN = {
        .name = "puts",
        .name_len = 4,
        .type = AuFnRef,
        .fn.fn_ref = au_io_puts,
        .params_len = 1,
};

static const AuVar AU_NIL = {.type = AuNil};
static const AuVar AU_TRUE = {.type = AuBool, .data = {.bool_val = true}};
static const AuVar AU_FALSE = {.type = AuBool, .data = {.bool_val = false}};

#endif
