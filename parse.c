#include "parse.h"

#include "runtime.h"
#include "util.h"

#define ASSERT_SEQ_TK(n, ...) __assert_seq_tk(tokens, (AuTokenType[]){__VA_ARGS__}, n, i, line)

#define ASSERT_WL(cond, fmt, ...) ASSERT(cond, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, line)
#define ERR_WL(fmt, ...)                                                                                               \
    fprintf(stderr, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, line);                                             \
    exit(1)

int __assert_seq_tk(const AuToken *all_tokens, const AuTokenType *wanted_tokens, const int wanted_tokens_len,
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

int __get_token_pos_after(const AuToken *tokens, const int tokens_len, const int current, const AuTokenType type)
{
    for (int i = current; i < tokens_len; ++i)
    {
        if (tokens[i].type == type)
        {
            return i;
        }
    }
    return -1;
}

AuVar __invoke_named_function(const AuModule *module, const char *function_name, AuVar *args, const int args_len,
                              const int line)
{
    const AuFunction *fn = au_get_function(module, function_name);
    ASSERT_WL(fn, "Function %s does not exist", function_name);
    ASSERT_WL(fn->params_len == args_len, "%d != %d. Incorrect argument count for function %s", args_len,
              fn->params_len, fn->name);

    return fn->fn(args);
}

void __parse_function_args(const AuRuntime *runtime, const AuToken *tokens, const int line, const int start,
                           const int end, AuVar *out_args, int *out_len)
{
    *out_len = 0;

    if (tokens[start].type == AuTkOpenParen)
    {
        return;
    }

    int arg_start = start;
    int closes_needed = 1;
    for (int i = start; i <= end; ++i)
    {
        if (tokens[i].type == AuTkOpenParen)
        {
            ++closes_needed;
        }
        else if (tokens[i].type == AuTkComma)
        {
            out_args[(*out_len)++] = parse_expr(runtime, tokens, line, arg_start, i - 1);
            arg_start = i + 1;
        }
        else if (tokens[i].type == AuTkCloseParen && --closes_needed == 0)
        {
            if (i - arg_start > 0)
            {
                out_args[(*out_len)++] = parse_expr(runtime, tokens, line, arg_start, i - 1);
            }
            break;
        }
    }
}

AuVar parse_expr(const AuRuntime *runtime, const AuToken *tokens, const int line, const int start, const int end)
{
    switch (tokens[start].type)
    {
        case AuTkString:
            return (AuVar){
                    .type = AuString,
                    .data.string_val = tokens[start].data.string_data.data,
            };
        case AuTkLitTrue:
        case AuTkLitFalse:
            return (AuVar){
                    .type = AuBool,
                    .data.bool_val = tokens[start].type == AuTkLitTrue,
            };
        case AuTkIdent:
            const char *ident_name = tokens[start].data.ident_data.data;

            const AuModule *module = au_get_module(runtime, ident_name);
            if (module)
            {
                __assert_seq_tk(tokens, (AuTokenType[]){AuTkFullStop, AuTkIdent}, 2, start, line);

                int args_len;
                AuVar args[64];
                __parse_function_args(runtime, tokens, line, start + 4, end, args, &args_len);

                const char *function_name = tokens[start + 2].data.ident_data.data;
                return __invoke_named_function(module, function_name, args, args_len, line);
            }

            __assert_seq_tk(tokens, (AuTokenType[]){AuTkIdent}, 1, start, line);

            int args_len;
            AuVar args[64];
            __parse_function_args(runtime, tokens, line, start + 2, end, args, &args_len);

            return __invoke_named_function(&runtime->local, ident_name, args, args_len, line);
        default:
            ERR_WL("Invalid start to expression");
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
                const int then_i = __get_token_pos_after(tokens, tokens_len, i, AuTkThen);

                const AuVar expr_res = parse_expr(runtime, tokens, line, i + 1, then_i - 1);
                ASSERT_WL(expr_res.type == AuBool, "If statement condition must result in a boolean");

                int end_i = then_i + 1;
                int ends_needed = 1;
                for (; end_i < tokens_len; ++end_i)
                {
                    if (tokens[end_i].type == AuTkIf && tokens[end_i - 2].type != AuTkElse)
                    {
                        ++ends_needed;
                        continue;
                    }
                    if (tokens[end_i].type == AuTkEnd && --ends_needed == 0)
                    {
                        break;
                    }
                }

                ASSERT_WL(tokens[end_i].type == AuTkEnd, "If statement was not closed");

                const int else_i = __get_token_pos_after(tokens, tokens_len, then_i, AuTkElse);
                if (expr_res.data.bool_val)
                {
                    const int if_block_len = (else_i > 0 ? else_i : end_i) - then_i;
                    parse(runtime, &tokens[then_i + 1], if_block_len);
                }
                else if (else_i > 0)
                {
                    parse(runtime, &tokens[else_i + 1], end_i - else_i);
                }

                i = end_i;

                break;
            case AuTkIdent:
                const int newline_i = __get_token_pos_after(tokens, tokens_len, i, AuTkNewline);

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
