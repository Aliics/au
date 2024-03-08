#include "token.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "util.h"

void __append_token(AuToken **tokens, int *token_len, const AuToken new_token)
{
    *token_len += 1;
    AuToken *tmp = realloc(*tokens, sizeof(AuToken) * *token_len);
    ASSERT(tmp, "Could not add token %d", new_token.type);

    *tokens = tmp;
    (*tokens)[*token_len - 1] = new_token;
}

AuToken *au_build_tokens(const char *src, const int src_len, int *out_len)
{
    *out_len = 0;
    if (src_len == 0)
        return NULL;

    AuToken *tokens = malloc(0);

    bool in_string = false;
    int string_len = 0;
    char string[AU_MAX_STRING_LEN];

    int ident_len = 0;
    char ident[AU_MAX_IDENT_LEN];
    for (int i = 0; i < src_len; ++i)
    {
        if (src[i] == AU_STRING_QUOTE)
        {
            if (in_string)
            {
                __append_token(&tokens, out_len,
                               (AuToken){
                                       .type = AuTkString,
                                       .data.string_data = make_string_data(string, string_len),
                               });

                string_len = 0;
            }

            in_string = !in_string;

            continue;
        }

        if (in_string)
        {
            ASSERT(string_len < AU_MAX_STRING_LEN, "String cannot be more than %d characters", AU_MAX_STRING_LEN);

            string[string_len++] = src[i];
            continue;
        }

        bool has_match = false;
        for (int j = 0; j < LENGTH(token_matches); ++j)
        {
            const AuTokenMatch match = token_matches[j];
            const int lit_len = strlen(match.literal);
            if (strncmp(&src[i], match.literal, lit_len) != 0)
                continue;

            if (!match.can_terminate_literal && ident_len > 0)
            {
                goto continue_literal;
            }

            __append_token(&tokens, out_len, (AuToken){.type = match.type});
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

            __append_token(&tokens, out_len,
                           (AuToken){
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
                    __append_token(&tokens, out_len, (AuToken){.type = AuTkComment});
                    --i; // Need to decrement so we aren't set to the next char.
                    break;
                }
            }
            goto clear_ident;
        }

    continue_literal:
        ASSERT(ident_len < AU_MAX_IDENT_LEN, "Identifier cannot exceed %d characters", AU_MAX_IDENT_LEN);

        ident[ident_len++] = src[i];
        continue;

    clear_ident:
        if (ident_len)
        {
            __append_token(&tokens, out_len,
                           (AuToken){
                                   .type = AuTkIdent,
                                   .data.ident_data = make_string_data(ident, ident_len),
                           });

            // Swap ident with the newly added token.
            const AuToken tmp = tokens[*out_len - 1];
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

char *au_token_type_string(const AuTokenType type)
{
    switch (type)
    {
        case AuTkComment:
            return AU_TK_COMMENT;
        case AuTkNewline:
            return "(newline)";
        case AuTkWhitespace:
            return "(whitespace)";
        case AuTkFullStop:
            return AU_TK_FULLSTOP;
        case AuTkComma:
            return AU_TK_COMMA;
        case AuTkIdent:
            return "(ident)";
        case AuTkString:
            return "(string)";
        case AuTkOpenParen:
            return AU_TK_OPEN_PAREN;
        case AuTkCloseParen:
            return AU_TK_CLOSE_PAREN;
        case AuTkAssign:
            return AU_TK_ASSIGN;
        case AuTkEquals:
            return AU_TK_EQUALS;
        case AuTkIf:
            return AU_TK_IF;
        case AuTkThen:
            return AU_TK_THEN;
        case AuTkElse:
            return AU_TK_ELSE;
        case AuTkEnd:
            return AU_TK_END;
        case AuTkUsing:
            return AU_TK_USING;
        case AuTkDef:
            return AU_TK_DEF;
        case AuTkAnd:
            return AU_TK_AND;
        case AuTkOr:
            return AU_TK_OR;
        case AuTkLitNil:
            return AU_TK_LIT_NIL;
        case AuTkLitTrue:
            return AU_TK_LIT_TRUE;
        case AuTkLitFalse:
            return AU_TK_LIT_FALSE;
        default:
            fprintf(stderr, "Cannot stringify unknown token");
            exit(1);
    }
}
