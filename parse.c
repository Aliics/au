#include "parse.h"

#include "runtime.h"
#include "util.h"

#define ASSERT_SEQ_TK(n, ...) assert_seq_tk(tokens, (AuTokenType[]){__VA_ARGS__}, n, i, line)

#define ASSERT_WL(cond, fmt, ...) ASSERT(cond, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, line)
#define ERR_WL(fmt, ...)                                                                                               \
    fprintf(stderr, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, line);                                             \
    exit(1);

int assert_seq_tk(const AuToken *all_tokens, const AuTokenType *wanted_tokens, const int wanted_tokens_len,
                  const int current, const int line)
{
    AuTokenType last = all_tokens[current].type;
    for (int i = 0; i < wanted_tokens_len; ++i)
    {
        const AuTokenType tk_type = all_tokens[current + i + 1].type;
        const AuTokenType wanted_tk_type = wanted_tokens[i];
        ASSERT_WL(wanted_tk_type == tk_type, "Expected \"%s\" after \"%s\" but got \"%s\"",
                  au_token_type_string(wanted_tk_type), au_token_type_string(last), au_token_type_string(tk_type));
        last = all_tokens[current + i + 1].type;
    }

    return wanted_tokens_len;
}

int walk_to_token_i(const AuToken *tokens, const int tokens_len, const int current, const AuTokenType type)
{
    int i = current;
    for (; i < tokens_len; ++i)
    {
        if (tokens[i].type == type)
            break;
    }
    return i;
}

AuVar parse_expr(const AuRuntime *runtime, const AuToken *tokens, const int line, const int start, const int end)
{
    switch (tokens[start].type)
    {
        case AuTkIdent:
            const char *ident_name = tokens[start].data.ident_data.data;

            const AuModule *module = au_get_module(runtime, ident_name);
            if (module)
            {
                assert_seq_tk(tokens, (AuTokenType[]){AuTkFullStop, AuTkIdent}, 2, start, line);

                int args_len = 0;
                AuVar args[64];
                for (int i = start + 3; i < end; ++i)
                {
                    switch (tokens[i].type)
                    {
                        case AuTkString:
                            args[args_len].type = AuString;
                            args[args_len].data.string_val = tokens[i].data.string_data.data;
                            ++args_len;
                            break;
                        default:
                            break;
                    }
                }

                const char *function_name = tokens[start + 2].data.ident_data.data;
                return au_invoke_named_function(module, function_name, args, args_len);
            }

            assert_seq_tk(tokens, (AuTokenType[]){AuTkIdent}, 1, start, line);

            return au_invoke_named_function(&runtime->local, ident_name, NULL, 0);
        default:
            ERR_WL("Invalid start to expression")
    }
}

void parse(const AuRuntime *runtime, const AuToken *tokens, const int tokens_len)
{
    unsigned int line = 1;

    for (int i = 0; i < tokens_len; ++i)
    {
        switch (tokens[i].type)
        {
            case AuTkNewline:
                ++line;
                break;
            case AuTkUsing:
                i += ASSERT_SEQ_TK(3, AuTkWhitespace, AuTkIdent, AuTkNewline);
                ++line;
                break;
            case AuTkIf:
                i += ASSERT_SEQ_TK(1, AuTkWhitespace);
                const int then_i = walk_to_token_i(tokens, tokens_len, i, AuTkThen);

                const AuVar expr_res = parse_expr(runtime, tokens, line, i + 1, then_i - 1);
                ASSERT_WL(expr_res.type == AuBool, "If statement condition must result in a boolean");

                i = then_i;

                break;
            case AuTkIdent:
                const int newline_i = walk_to_token_i(tokens, tokens_len, i, AuTkNewline);

                parse_expr(runtime, tokens, line, i, newline_i - 1);

                i = newline_i;

                break;
            case AuTkWhitespace:
            case AuTkElse:
            case AuTkEnd:
                break;
            case AuTkComment:
                // Ignore comments.
                break;
            default:
                fprintf(stderr, "Unexpected token %s\n", au_token_type_string(tokens[i].type));
                exit(1);
        }
    }
}