#include "runtime.h"

#include <string.h>

AuModule *au_get_module(const AuRuntime runtime, const char *name)
{
    for (int i = 0; i < runtime.modules_len; ++i)
    {
        const AuModule mod = runtime.modules[i];
        if (strncmp(mod.name, name, mod.name_len) != 0)
        {
            continue;
        }

        return &runtime.modules[i];
    }

    return NULL;
}

AuFunction *au_get_function(const AuModule *module, const char *name)
{
    for (int i = 0; i < module->functions_len; ++i)
    {
        const AuFunction fn = module->functions[i];
        if (strncmp(name, fn.name, fn.name_len) != 0)
        {
            continue;
        }

        return &module->functions[i];
    }

    return NULL;
}


AuVarDef *au_get_variable(const AuModule *module, const char *name)
{
    for (int i = 0; i < module->variables_len; ++i)
    {
        const AuVarDef var = module->variables[i];
        if (strncmp(name, var.name, var.name_len) != 0)
        {
            continue;
        }

        return &module->variables[i];
    }

    return NULL;
}
