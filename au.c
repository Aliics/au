#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "parse.h"
#include "std_inlined.h"
#include "token.h"
#include "util.h"

static const AuRuntime runtime = {
        .modules =
                (AuModule[]){
                        {.name = "os",
                         .name_len = 2,
                         .functions =
                                 (AuFunction[]){
                                         {.name = "is_linux?", .name_len = 9, .fn = au_os_is_linux, .params_len = 0},
                                         {.name = "is_macos?", .name_len = 9, .fn = au_os_is_macos, .params_len = 0},
                                         {.name = "is_windows?",
                                          .name_len = 1,
                                          .fn = au_os_is_windows,
                                          .params_len = 0},
                                 },
                         .functions_len = 3},
                        {.name = "io",
                         .name_len = 2,
                         .functions =
                                 (AuFunction[]){
                                         {.name = "puts", .name_len = 4, .fn = au_io_puts, .params_len = 1},
                                 },
                         .functions_len = 1}},
        .modules_len = 2};

int main(const int arg_len, char **args)
{
    ASSERT(arg_len > 1, "Expected one file as a program argument\n");

    int src_len;
    char *src = read_file(args[1], &src_len);

    int tokens_len;
    AuToken *tokens = au_build_tokens(src, src_len, &tokens_len);
    // for (int i = 0; i < tokens_len; ++i)
    // {
    //     printf("%s\n", au_token_type_string(tokens[i].type));
    // }

    free(src);

    parse(&runtime, tokens, tokens_len);

    free(tokens);
}
