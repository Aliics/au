#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "parse.h"
#include "std_inlined.h"
#include "token.h"
#include "util.h"

static AuRuntime runtime = {
        .local =
                {
                        .functions = (AuFunction[1 << 16]){},
                        .functions_len = 0,
                        .variables = (AuVarDef[1 << 16]){},
                        .variables_len = 0,
                },
        .modules =
                (AuModule[]){
                        {
                                .name = "os",
                                .name_len = 2,
                                .functions =
                                        (AuFunction[]){
                                                AU_OS_IS_LINUX_FN,
                                                AU_OS_IS_MACOS_FN,
                                                AU_OS_IS_WINDOWS_FN,
                                        },
                                .functions_len = 3,
                        },
                        {
                                .name = "io",
                                .name_len = 2,
                                .functions = (AuFunction[]){AU_IO_PUTS_FN},
                                .functions_len = 1,
                        },
                },
        .modules_len = 2,
};

int main(const int arg_len, char **args)
{
    ASSERT(arg_len > 1, "Expected one file as a program argument");

    int src_len;
    char *src = read_file(args[1], &src_len);

    int tokens_len;
    AuToken *tokens = au_build_tokens(src, src_len, &tokens_len);
    // for (int i = 0; i < tokens_len; ++i)
    // {
    //     printf("%s\n", au_token_type_string(tokens[i].type));
    // }

    free(src);

    AuParseCtx ctx = {
            .rt = &runtime,
            .tks = tokens,
            .tks_len = tokens_len,
            .ln = 1,
    };

    parse(&ctx, int_range(0, tokens_len - 1), 0);

    free(tokens);
}
