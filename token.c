#include "token.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "util.h"

void append_token(AUToken **tokens, int *token_len, const AUToken new_token)
{
    *token_len += 1;
    AUToken *tmp = realloc(*tokens, sizeof(AUToken) * *token_len);
    ASSERT(tmp, "Could not add token %d", new_token.type);

    *tokens = tmp;
    (*tokens)[*token_len - 1] = new_token;
}

AUToken *au_build_tokens(const char *src, const int src_len, int *out_len)
{
    *out_len = 0;
    if (src_len == 0)
        return NULL;

    AUToken *tokens = malloc(0);

    bool in_string = false;
    int string_len = 0;
    char string[AU_MAX_STRING_LEN];

    int ident_len = 0;
    char ident[AU_MAX_IDENT_LEN];
    for (int i = 0; i < src_len; ++i)
    {
        if (src[i] == AU_STRING_QUOTE)
        {
            in_string = !in_string;
            if (!in_string)
                continue;

            append_token(&tokens, out_len,
                         (AUToken){
                                 .type = AuTkString,
                                 .data.string_data = make_string_data(string, string_len),
                         });

            string_len = 0;

            continue;
        }

        if (in_string)
        {
            ASSERT(string_len < AU_MAX_STRING_LEN, "String cannot be more than %d characters", AU_MAX_STRING_LEN);

            string[string_len++] = src[i];
            continue;
        }

        bool has_match = false;
        for (int j = 0; j < LENGTH(token_literals); ++j)
        {
            const AUTokenMatch match = token_literals[j];
            const int lit_len = strlen(match.literal);
            if (strncmp(&src[i], match.literal, lit_len) != 0)
                continue;

            append_token(&tokens, out_len, (AUToken){.type = match.type});
            i += lit_len - 1;
            has_match = true;
            break;
        }
        if (has_match)
            goto clear_ident;

        if (src[i] == AU_TK_WHITESPACE[0])
        {
            int ws_len = 0;
            for (; i < src_len && src[i] == ' '; ++i)
            {
                ws_len++;
            }
            --i; // Need to decrement so we aren't set to the next char.

            append_token(&tokens, out_len,
                         (AUToken){
                                 .type = AuTkWhitespace,
                                 .data.whitespace_len = ws_len,
                         });
            goto clear_ident;
        }

        if (strncmp(&src[i], AU_TK_COMMENT, 2) == 0)
        {
            for (; i < src_len; ++i)
            {
                if (src[i] == AU_TK_NEWLINE[0])
                {
                    append_token(&tokens, out_len, (AUToken){.type = AuTkComment});
                    --i; // Need to decrement so we aren't set to the next char.
                    break;
                }
            }
            goto clear_ident;
        }

        ASSERT(ident_len < AU_MAX_IDENT_LEN, "Identifier cannot exceed %d characters", AU_MAX_IDENT_LEN);

        ident[ident_len++] = src[i];
        continue;

    clear_ident:
        if (ident_len)
        {
            append_token(&tokens, out_len,
                         (AUToken){
                                 .type = AuTkIdent,
                                 .data.ident_data = make_string_data(ident, ident_len),
                         });

            // Swap ident with the newly added token.
            const AUToken tmp = tokens[*out_len - 1];
            tokens[*out_len - 1] = tokens[*out_len - 2];
            tokens[*out_len - 2] = tmp;
        }
        ident_len = 0;
    }

    return tokens;
}

AUStringData make_string_data(const char *str, const int len)
{
    AUStringData string_data;
    string_data.len = len;

    string_data.data = malloc(len + 1);
    strncpy(string_data.data, str, len);
    string_data.data[len] = '\0';

    return string_data;
}
