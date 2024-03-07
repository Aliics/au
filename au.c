#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "file.h"
#include "token.h"

int main(const int arg_len, char **args)
{
    ASSERT(arg_len > 1, "Expected one file as a program argument");

    int src_len;
    char *src = read_file(args[1], &src_len);

    printf("%s", src);

    int tokens_len;
    AUToken *tokens = au_build_tokens(src, src_len, &tokens_len);
    for (int i = 0; i < tokens_len; ++i)
    {
        char *meta = "(nil)";
        const AUToken token = tokens[i];
        if (token.type == AuTkWhitespace)
        {
            meta = (char[10]){};
            sprintf(meta, "%d ws", token.data.whitespace_len);
        }
        else if (token.type == AuTkIdent)
        {
            meta = token.data.ident_data.data;
        }
        else if (token.type == AuTkString)
        {
            meta = token.data.string_data.data;
        }
        printf("%d: %s\n", token.type, meta);
    }

    free(tokens);
    free(src);
}
