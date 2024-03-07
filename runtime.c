#include "runtime.h"

#include <string.h>

#include "util.h"

AuModule *au_get_module(const AuRuntime *runtime, const char *name)
{
    for (int i = 0; i < runtime->modules_len; ++i)
    {
        const AuModule mod = runtime->modules[i];
        if (strncmp(mod.name, name, mod.name_len) != 0)
        {
            continue;
        }

        return &runtime->modules[i];
    }

    return NULL;
}

AuFunction *au_get_function(const AuModule *module, const char *name)
{
    for (int i = 0; i < module->functions_len; ++i)
    {
        const AuFunction fn = module->functions[i];
        if (strncmp(fn.name, name, fn.name_len) != 0)
        {
            continue;
        }

        return &module->functions[i];
    }

    return NULL;
}

AuVar au_invoke_named_function(const AuModule *module, const char *function_name, AuVar *args, const int args_len)
{
    const AuFunction *fn = au_get_function(module, function_name);
    ASSERT(fn, "Function %s does not exist\n", function_name);
    ASSERT(fn->params_len == args_len, "%d != %d. Incorrect argument count for function %s\n", args_len, fn->params_len,
           fn->name);

    return fn->fn(args);
}
