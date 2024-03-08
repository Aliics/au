#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "parse.h"
#include "std_inlined.h"
#include "token.h"
#include "util.h"

static AuRuntime runtime = {
        .local = {.functions = (AuFunction[1024]){}, .functions_len = 0},
        .modules =
                (AuModule[]){
                        {
                                .name = "os",
                                .name_len = 2,
                                .functions =
                                        (AuFunction[]){
                                                au_os_is_linux_function,
                                                au_os_is_macos_function,
                                                au_os_is_windows_function,
                                        },
                                .functions_len = 3,
                        },
                        {
                                .name = "io",
                                .name_len = 2,
                                .functions = (AuFunction[]){au_io_puts_function},
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

    parse(&runtime, tokens, tokens_len, 0);

    free(tokens);
}
