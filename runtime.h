#ifndef AU_RUNTIME_H
#define AU_RUNTIME_H

#include <stdbool.h>

#include "util.h"

#define AU_MAX_FUNCTION_PARAMS 64

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

typedef enum AuFunctionType
{
    AuFnProgramTokens,
    AuFnRef,
} AuFunctionType;

typedef struct AuFunctionDef
{
    int starting_line;
    IntRange token_range;
} AuFunctionDef;

typedef struct AuFunction
{
    char *name;
    int name_len;

    char *params[AU_MAX_FUNCTION_PARAMS];
    int params_len;

    AuFunctionType type;
    union
    {
        AuFunctionDef fn_prog_tokens;
        AuVar (*fn_ref)(AuVar *);
    } fn;
} AuFunction;

typedef struct AuVarDef
{
    char *name;
    int name_len;

    AuVar var;
} AuVarDef;

typedef struct AuModule
{
    char *name;
    int name_len;

    AuFunction *functions;
    int functions_len;

    AuVarDef *variables;
    int variables_len;
} AuModule;

typedef struct AuRuntime
{
    AuModule *modules;
    int modules_len;

    AuModule local;
} AuRuntime;



AuModule *au_get_module(AuRuntime, const char *);

AuFunction *au_get_function(const AuModule *, const char *);

AuVarDef *au_get_variable(const AuModule *, const char *);

#endif
