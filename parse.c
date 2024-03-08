#include "parse.h"

#include "runtime.h"
#include "util.h"

#define ASSERT_WL(cond, fmt, ...) ASSERT(cond, fmt " (line: %d)" __VA_OPT__(, ) __VA_ARGS__, line)
#define ERR_WL(fmt, ...)                                                                                               \
    fprintf(stderr, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, line);                                             \
    exit(1)

void __walk_token_sequence(const AuToken *all_tokens, const AuTokenType *wanted_tokens, const int wanted_tokens_len,
                           const int line, int *out_i)
{
    AuTokenType last = all_tokens[*out_i].type;
    for (int i = 0; i < wanted_tokens_len; ++i, ++*out_i)
    {
        const AuTokenType tk_type = all_tokens[*out_i + 1].type;
        const AuTokenType wanted_tk_type = wanted_tokens[i];
        ASSERT_WL(wanted_tk_type == tk_type, "Expected \"%s\" after \"%s\" but got \"%s\"",
                  au_token_type_string(wanted_tk_type), au_token_type_string(last), au_token_type_string(tk_type));
        last = all_tokens[*out_i + 1].type;
    }
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

int __get_end_block_pos(const AuToken *tokens, const int tokens_len, const int start)
{
    int ends_needed = 1;
    for (int i = start; i < tokens_len; ++i)
    {
        if (tokens[i].type == AuTkIf && tokens[i - 2].type != AuTkElse)
        {
            ++ends_needed;
            continue;
        }
        if (tokens[i].type == AuTkEnd && --ends_needed == 0)
        {
            return i;
        }
    }

    return -1;
}

AuVar __invoke_named_function(AuRuntime *runtime, const AuToken *tokens, const AuModule *module,
                              const char *function_name, AuVar *args, const int args_len, const int line)
{
    const AuFunction *fn = au_get_function(module, function_name);
    ASSERT_WL(fn, "Function %s does not exist", function_name);
    ASSERT_WL(fn->params_len == args_len, "%d != %d. Incorrect argument count for function %s", args_len,
              fn->params_len, fn->name);

    if (fn->type == AuFnRef)
    {
        return fn->fn.fn_ref(args);
    }

    const AuFunctionProgramTokens fpt = fn->fn.fn_prog_tokens;
    return parse(runtime, &tokens[fpt.token_range.start], fpt.token_range.end - fpt.token_range.start,
                 fpt.starting_line);
}

void __parse_function_args(AuRuntime *runtime, const AuToken *tokens, const int line, const IntRange args_range,
                           AuVar *out_args, int *out_len, int *out_i)
{
    *out_len = 0;
    *out_i = args_range.start;

    if (tokens[args_range.start].type == AuTkOpenParen)
    {
        return;
    }

    int arg_start = args_range.start;
    int closes_needed = 1;
    for (int i = args_range.start; i <= args_range.end; ++i)
    {
        if (tokens[i].type == AuTkOpenParen)
        {
            ++closes_needed;
        }
        else
        {
            const IntRange arg_range = int_range(arg_start, i - 1);

            if (tokens[i].type == AuTkComma)
            {
                out_args[(*out_len)++] = parse_expr(runtime, tokens, line, arg_range);
                arg_start = i + 1;
            }
            else if (tokens[i].type == AuTkCloseParen && --closes_needed == 0)
            {
                if (i - arg_start > 0)
                {
                    out_args[(*out_len)++] = parse_expr(runtime, tokens, line, arg_range);
                }

                *out_i = i;
                break;
            }
        }
    }
}

AuVar __parse_with_bool_operator(AuRuntime *runtime, const AuToken *tokens, const int line,
                                 const AuTokenType operator_type, const AuVar left_eval,
                                 const IntRange right_expr_range)
{
    ASSERT_WL(left_eval.type == AuBool, "Expression with %s must evaluate be a bool, got %s",
              au_token_type_string(operator_type), au_token_type_string(left_eval.type));

    if (left_eval.data.bool_val == (operator_type == AuTkOr))
    {
        return left_eval;
    }

    const AuVar right_eval = parse_expr(runtime, tokens, line, int_range(right_expr_range.start, right_expr_range.end));
    ASSERT_WL(left_eval.type == AuBool, "Expression with %s must evaluate be a bool, got %s",
              au_token_type_string(operator_type), au_token_type_string(left_eval.type));

    return right_eval;
}

AuVar parse_expr(AuRuntime *runtime, const AuToken *tokens, const int line, const IntRange expr_range)
{
    bool has_evaled = false;
    AuVar eval = {.type = AuNil};

    for (int i = expr_range.start; i <= expr_range.end; ++i)
    {
        switch (tokens[i].type)
        {
            case AuTkAnd:
            case AuTkOr:
                ASSERT_WL(has_evaled, "\"and\" cannot appear before expression");
                return __parse_with_bool_operator(runtime, tokens, line, tokens[i].type, eval,
                                                  int_range(i + 1, expr_range.end));
            case AuTkString:
                has_evaled = true;
                eval = (AuVar){
                        .type = AuString,
                        .data.string_val = tokens[i].data.string_data.data,
                };
                break;
            case AuTkLitTrue:
            case AuTkLitFalse:
                has_evaled = true;
                eval = (AuVar){
                        .type = AuBool,
                        .data.bool_val = tokens[i].type == AuTkLitTrue,
                };
                break;
            case AuTkIdent:
            {
                const char *ident_name = tokens[i].data.ident_data.data;

                const AuModule *module = au_get_module(runtime, ident_name);
                if (module)
                {
                    __walk_token_sequence(tokens, (AuTokenType[]){AuTkFullStop, AuTkIdent}, 2, line, &i);

                    const char *function_name = tokens[i].data.ident_data.data;

                    int args_len;
                    AuVar args[64];
                    __parse_function_args(runtime, tokens, line, int_range(i + 2, expr_range.end), args, &args_len, &i);

                    has_evaled = true;
                    eval = __invoke_named_function(runtime, tokens, module, function_name, args, args_len, line);
                    break;
                }

                int args_len;
                AuVar args[64];
                __parse_function_args(runtime, tokens, line, int_range(i + 1, expr_range.end), args, &args_len, &i);

                has_evaled = true;
                eval = __invoke_named_function(runtime, tokens, &runtime->local, ident_name, args, args_len, line);
                break;
            }
            case AuTkWhitespace:
                break; // Do nothing.
            default:
                ERR_WL("Invalid start to expression %s", au_token_type_string(tokens[i].type));
        }
    }

    return eval;
}

AuVar parse(AuRuntime *runtime, const AuToken *tokens, const int tokens_len, const int starting_line)
{
    AuVar eval = {.type = AuNil};

    unsigned int line = starting_line;

    for (int i = 0; i < tokens_len; ++i)
    {
        switch (tokens[i].type)
        {
            case AuTkNewline:
                ++line;
                break;
            case AuTkUsing:
                __walk_token_sequence(tokens, (AuTokenType[]){AuTkWhitespace, AuTkIdent}, 2, line, &i);
                ++line;
                break;
            case AuTkIf:
            {
                __walk_token_sequence(tokens, (AuTokenType[]){AuTkWhitespace}, 1, line, &i);

                const int then_i = __get_token_pos_after(tokens, tokens_len, i, AuTkThen);

                const AuVar expr_res = parse_expr(runtime, tokens, line, int_range(i + 1, then_i - 1));
                ASSERT_WL(expr_res.type == AuBool, "If statement condition must result in a boolean");

                const int if_end_i = __get_end_block_pos(tokens, tokens_len, then_i + 1);

                ASSERT_WL(i > 0, "If statement was not closed");

                const int else_i = __get_token_pos_after(tokens, tokens_len, then_i, AuTkElse);
                if (expr_res.data.bool_val)
                {
                    const int if_block_len = (else_i > 0 ? else_i : if_end_i) - then_i;
                    parse(runtime, &tokens[then_i + 1], if_block_len, line);
                }
                else if (else_i > 0)
                {
                    parse(runtime, &tokens[else_i + 1], if_end_i - else_i, line);
                }

                i = if_end_i;

                break;
            }
            case AuTkDef:
            {
                __walk_token_sequence(tokens, (AuTokenType[]){AuTkWhitespace, AuTkIdent}, 2, line, &i);

                const int def_end_i = __get_end_block_pos(tokens, tokens_len, i);

                const AUStringData ident_data = tokens[i - 1].data.ident_data;
                const AuFunctionProgramTokens fpt = {
                        .starting_line = line,
                        .token_range = int_range(i + 1, def_end_i - 1),
                };
                runtime->local.functions[runtime->local.functions_len++] = (AuFunction){
                        .name = ident_data.data,
                        .name_len = ident_data.len,
                        .type = AuFnProgramTokens,
                        .fn = fpt,
                };

                i = def_end_i;

                break;
            }
            case AuTkWhitespace:
            case AuTkElse:
            case AuTkEnd:
                break;
            case AuTkComment:
                // Ignore comments.
                break;
            default:
            {
                const int newline_i = __get_token_pos_after(tokens, tokens_len, i, AuTkNewline);
                const int end = newline_i > 0 ? newline_i : tokens_len;

                eval = parse_expr(runtime, tokens, line, int_range(i, end - 1));

                i = end;
                break;
            }
        }
    }

    return eval;
}
