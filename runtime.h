#ifndef AU_RUNTIME_H
#define AU_RUNTIME_H

#include <stdbool.h>

typedef enum AuVarType
{
    AuNil,
    AuBool,
    AuInt,
    AuString,
} AuVarType;

typedef struct AuVar
{
    AuVarType type;
    union
    {
        bool bool_val;
        int int_val;
        char *string_val;
    } data;
} AuVar;

typedef struct AuFunction
{
    char *name;
    int name_len;

    int params_len;
    AuVar (*fn)(AuVar *);
} AuFunction;

typedef struct AuModule
{
    char *name;
    int name_len;

    AuFunction *functions;
    int functions_len;

    AuVar *variables;
    int variables_len;
} AuModule;

typedef struct AuRuntime
{
    AuModule *modules;
    int modules_len;

    AuModule local;
} AuRuntime;

static const AuVar au_static_nil = {.type = AuNil};
static const AuVar au_static_true = {.type = AuBool, .data = {.bool_val = true}};
static const AuVar au_static_false = {.type = AuBool, .data = {.bool_val = false}};

AuModule *au_get_module(const AuRuntime *, const char *);

AuFunction *au_get_function(const AuModule *, const char *);

AuVar au_invoke_named_function(const AuModule *, const char *, AuVar *, int);

#endif
