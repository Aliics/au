#include "parse.h"

#include <string.h>

#include "runtime.h"
#include "util.h"

#define ASSERT_WCTX(cond, fmt, ...) ASSERT(cond, fmt " (line: %d)" __VA_OPT__(, ) __VA_ARGS__, ctx->ln)
#define ERR_WCTX(fmt, ...)                                                                                             \
    fprintf(stderr, fmt " (line: %d)\n" __VA_OPT__(, ) __VA_ARGS__, ctx->ln);                                          \
    exit(1)

void __walk_token_sequence(const AuParseCtx *ctx, const AuTokenType *wanted_tokens, const int wanted_tokens_len,
                           int *out_i)
{
    AuTokenType last = ctx->tks[*out_i].type;
    for (int i = 0; i < wanted_tokens_len; ++i, ++*out_i)
    {
        const AuTokenType tk_type = ctx->tks[*out_i + 1].type;
        const AuTokenType wanted_tk_type = wanted_tokens[i];
        ASSERT_WCTX(wanted_tk_type == tk_type, "Expected \"%s\" after \"%s\" but got \"%s\"",
                    au_token_type_to_string(wanted_tk_type), au_token_type_to_string(last),
                    au_token_type_to_string(tk_type));
        last = ctx->tks[*out_i + 1].type;
    }
}

int __get_token_pos_after(const AuParseCtx *ctx, const int current, const AuTokenType type)
{
    for (int i = current; i < ctx->tks_len; ++i)
    {
        if (ctx->tks[i].type == type)
        {
            return i;
        }
    }
    return -1;
}

int __get_end_block_pos(const AuParseCtx *ctx, const int start)
{
    int ends_needed = 1;
    for (int i = start; i < ctx->tks_len; ++i)
    {
        if (ctx->tks[i].type == AuTkIf && ctx->tks[i - 2].type != AuTkElse)
        {
            ++ends_needed;
            continue;
        }
        if (ctx->tks[i].type == AuTkEnd && --ends_needed == 0)
        {
            return i;
        }
    }

    return -1;
}

int __define_new_function(const AuParseCtx *ctx, const AuStringData ident_data, const int start)
{
    int i = start;
    AuFunction *new_function = &ctx->rt->local.functions[ctx->rt->local.functions_len++];

    if (ctx->tks[i + 1].type == AuTkOpenParen)
    {
        i += 2; // Skip into parameters list.
        for (; i < ctx->tks_len; ++i)
        {
            if (ctx->tks[i].type != AuTkIdent)
            {
                ERR_WCTX("Unexpected token in %s parameters list", ident_data.data);
            }

            new_function->params[new_function->params_len++] = ctx->tks[i].data.ident_data.data;

            if (ctx->tks[i + 1].type == AuTkComma)
            {
                ++i;
            }
            else if (ctx->tks[i + 1].type == AuTkCloseParen)
            {
                i += 2; // Skip out of parameters list.
                break;
            }
        }
    }
    else
    {
        ++i; // Just skip past signature.
    }

    const int def_starting_line = ctx->tks[i].type == AuTkNewline ? ctx->ln + 1 : ctx->ln;

    const int def_end_i = __get_end_block_pos(ctx, i);

    new_function->name = ident_data.data;
    new_function->name_len = ident_data.len;
    new_function->type = AuFnProgramTokens;
    new_function->fn.fn_prog_tokens = (AuFunctionDef){
            .starting_line = def_starting_line,
            .token_range = int_range(i + 1, def_end_i - 1),
    };

    return def_end_i;
}

AuVar parse(AuParseCtx *ctx, const IntRange tokens_range)
{
    AuVar eval = {.type = AuNil};

    for (int i = tokens_range.start; i < tokens_range.end; ++i)
    {
        switch (ctx->tks[i].type)
        {
            case AuTkNewline:
            {
                ++ctx->ln;
                break;
            }
            case AuTkUsing:
            {
                __walk_token_sequence(ctx, (AuTokenType[]){AuTkWhitespace, AuTkIdent}, 2, &i);
                ++ctx->ln;
                break;
            }
            case AuTkIf:
            {
                __walk_token_sequence(ctx, (AuTokenType[]){AuTkWhitespace}, 1, &i);

                const int then_i = __get_token_pos_after(ctx, i, AuTkThen);

                const AuVar expr_res = parse_expr(ctx, int_range(i + 1, then_i - 1));
                ASSERT_WCTX(expr_res.type == AuBool, "If statement condition must result in a boolean");

                const int if_end_i = __get_end_block_pos(ctx, then_i + 1);

                ASSERT_WCTX(i > 0, "If statement was not closed");

                const int else_i = __get_token_pos_after(ctx, then_i, AuTkElse);
                if (expr_res.data.bool_val)
                {
                    const int block_end = else_i > 0 && else_i < if_end_i ? else_i : if_end_i;
                    parse(ctx, int_range(then_i + 1, block_end));
                }
                else if (else_i > 0)
                {
                    parse(ctx, int_range(else_i + 1, if_end_i));
                }

                i = if_end_i;

                break;
            }
            case AuTkDef:
            {
                __walk_token_sequence(ctx, (AuTokenType[]){AuTkWhitespace, AuTkIdent}, 2, &i);

                const AuStringData ident_data = ctx->tks[i].data.ident_data;

                i = __define_new_function(ctx, ident_data, i);

                for (int j = i; j < i; ++j)
                {
                    if (ctx->tks[j].type == AuTkNewline)
                    {
                        ++ctx->ln;
                    }
                }

                break;
            }
            case AuTkWhitespace:
            case AuTkElse:
            case AuTkEnd:
            case AuTkComment:
                break;
            default:
            {
                const int newline_i = __get_token_pos_after(ctx, i, AuTkNewline);
                const int end = MIN(newline_i > 0 ? newline_i : ctx->tks_len, tokens_range.end);

                eval = parse_expr(ctx, int_range(i, end - 1));

                i = end;
                break;
            }
        }
    }

    return eval;
}

AuVar __invoke_named_function(AuParseCtx *ctx, AuModule *module, const char *function_name, AuVar *args,
                              const int args_len)
{
    const AuFunction *fn = au_get_function(module, function_name);
    ASSERT_WCTX(fn, "Function %s does not exist", function_name);
    ASSERT_WCTX(fn->params_len == args_len, "%d != %d. Incorrect argument count for function %s", args_len,
                fn->params_len, fn->name);

    if (fn->type == AuFnRef)
    {
        // Function reference. This has been inlined and just executes without additional checks.
        return fn->fn.fn_ref(args);
    }

    // Push all variables onto our stack before the function call.
    const int origin_variables_len = module->variables_len;
    const AuFunctionDef def = fn->fn.fn_prog_tokens;
    for (int i = 0; i < args_len; ++i)
    {
        module->variables[module->variables_len++] = (AuVarDef){
                .name = fn->params[i],
                .name_len = strlen(fn->params[i]),
                .var = args[i],
        };
    }

    // When calling a function, we need to change the "line" we are parsing.
    const int ret_ln = ctx->ln;
    ctx->ln = def.starting_line;

    const AuVar result = parse(ctx, def.token_range);
    ctx->ln = ret_ln;

    // Pop variables off afterwards. We don't want them.
    for (int i = origin_variables_len; i < module->variables_len; ++i)
    {
        module->variables[i] = (AuVarDef){};
    }
    module->variables_len = origin_variables_len;

    return result;
}

void __parse_function_args(AuParseCtx *ctx, const IntRange args_range, AuVar *out_args, int *out_len, int *out_i)
{
    *out_len = 0;
    *out_i = args_range.start;

    if (ctx->tks[args_range.start].type != AuTkOpenParen)
    {
        return;
    }

    int arg_start = args_range.start + 1;
    int closes_needed = 0;
    for (int i = args_range.start; i <= args_range.end; ++i)
    {
        if (ctx->tks[i].type == AuTkOpenParen)
        {
            ++closes_needed;
        }
        else
        {
            const IntRange arg_range = int_range(arg_start, i - 1);

            if (ctx->tks[i].type == AuTkComma)
            {
                out_args[(*out_len)++] = parse_expr(ctx, arg_range);
                arg_start = i + 1;
            }
            else if (ctx->tks[i].type == AuTkCloseParen && --closes_needed == 0)
            {
                if (i - arg_start > 0)
                {
                    out_args[(*out_len)++] = parse_expr(ctx, arg_range);
                }

                *out_i = i;
                break;
            }
        }
    }
}

AuVar __parse_with_bool_operator(AuParseCtx *ctx, const AuTokenType operator_type, const AuVar left_eval,
                                 const IntRange right_expr_range)
{
    ASSERT_WCTX(left_eval.type == AuBool, "Expression with %s must evaluate be a bool, got %s",
                au_token_type_to_string(operator_type), au_token_type_to_string(left_eval.type));

    if (left_eval.data.bool_val == (operator_type == AuTkOr))
    {
        return left_eval;
    }

    const AuVar right_eval = parse_expr(ctx, int_range(right_expr_range.start, right_expr_range.end));
    ASSERT_WCTX(left_eval.type == AuBool, "Expression with %s must evaluate be a bool, got %s",
                au_token_type_to_string(operator_type), au_token_type_to_string(left_eval.type));

    return right_eval;
}

AuVar parse_expr(AuParseCtx *ctx, const IntRange expr_range)
{
    bool has_evaled = false;
    AuVar eval = {.type = AuNil};

    for (int i = expr_range.start; i <= expr_range.end; ++i)
    {
        switch (ctx->tks[i].type)
        {
            case AuTkAnd:
            case AuTkOr:
            {
                ASSERT_WCTX(has_evaled, "%s cannot appear before expression",
                            au_token_type_to_string(ctx->tks[i].type));
                return __parse_with_bool_operator(ctx, ctx->tks[i].type, eval, int_range(i + 1, expr_range.end));
            }
            case AuTkNot:
            {
                const IntRange range_after_not = int_range(i + 1, expr_range.end);
                const AuVar right_expr = parse_expr(ctx, range_after_not);
                ASSERT_WCTX(right_expr.type == AuBool, "Expected bool expression after %s, got %s",
                            au_token_type_to_string(AuTkNot), au_token_type_to_string(right_expr.type));

                return (AuVar){
                        .type = AuBool,
                        .data.bool_val = !right_expr.data.bool_val,
                };
            }
            case AuTkString:
            {
                has_evaled = true;
                eval = (AuVar){
                        .type = AuString,
                        .data.string_val = ctx->tks[i].data.string_data.data,
                };
                break;
            }
            case AuTkLitTrue:
            case AuTkLitFalse:
            {
                has_evaled = true;
                eval = (AuVar){
                        .type = AuBool,
                        .data.bool_val = ctx->tks[i].type == AuTkLitTrue,
                };
                break;
            }
            case AuTkIdent:
            {
                const char *ident_name = ctx->tks[i].data.ident_data.data;

                AuModule *module = au_get_module(ctx->rt, ident_name);
                if (module)
                {
                    __walk_token_sequence(ctx, (AuTokenType[]){AuTkFullStop, AuTkIdent}, 2, &i);

                    const char *function_name = ctx->tks[i].data.ident_data.data;

                    int args_len;
                    AuVar args[AU_MAX_FUNCTION_PARAMS];
                    __parse_function_args(ctx, int_range(i + 1, expr_range.end), args, &args_len, &i);

                    has_evaled = true;
                    eval = __invoke_named_function(ctx, module, function_name, args, args_len);
                    break;
                }

                AuModule *local_module = &ctx->rt->local;

                const AuVarDef *local_var = au_get_variable(local_module, ident_name);
                if (local_var)
                {
                    has_evaled = true;
                    eval = local_var->var;
                    break;
                }

                int args_len;
                AuVar args[AU_MAX_FUNCTION_PARAMS];
                __parse_function_args(ctx, int_range(i + 1, expr_range.end), args, &args_len, &i);

                has_evaled = true;
                eval = __invoke_named_function(ctx, local_module, ident_name, args, args_len);
                break;
            }
            case AuTkWhitespace:
                break; // Do nothing.
            case AuTkComment:
                return eval; // Bye.
            default:
                ERR_WCTX("Invalid start to expression %s", au_token_type_to_string(ctx->tks[i].type));
        }
    }

    return eval;
}
