#ifndef AU_TOKEN_H
#define AU_TOKEN_H

#include <stdbool.h>

#define AU_TK_COMMENT "--"
#define AU_TK_NEWLINE "\n"
#define AU_TK_WHITESPACE " "
#define AU_TK_FULLSTOP "."
#define AU_TK_COMMA ","
#define AU_TK_OPEN_PAREN "("
#define AU_TK_CLOSE_PAREN ")"
#define AU_TK_ASSIGN "="
#define AU_TK_EQUALS "=="
#define AU_TK_IF "if"
#define AU_TK_THEN "then"
#define AU_TK_ELSE "else"
#define AU_TK_END "end"
#define AU_TK_USING "using"
#define AU_TK_DEF "def"
#define AU_TK_NOT "not"
#define AU_TK_AND "and"
#define AU_TK_OR "or"

#define AU_STRING_QUOTE '"'

#define AU_TK_LIT_NIL "nil"
#define AU_TK_LIT_TRUE "true"
#define AU_TK_LIT_FALSE "false"

#define AU_MAX_IDENT_LEN 1024
#define AU_MAX_STRING_LEN 65535

typedef enum AuTokenType
{
    AuTkComment,
    AuTkNewline,
    AuTkWhitespace,
    AuTkFullStop,
    AuTkComma,
    AuTkIdent,
    AuTkString,
    AuTkOpenParen,
    AuTkCloseParen,
    AuTkAssign,
    AuTkEquals,
    AuTkIf,
    AuTkThen,
    AuTkElse,
    AuTkEnd,
    AuTkUsing,
    AuTkDef,
    AuTkNot,
    AuTkAnd,
    AuTkOr,
    AuTkLitNil,
    AuTkLitTrue,
    AuTkLitFalse,
} AuTokenType;

typedef struct AUStringData
{
    char *data;
    int len;
} AuStringData;

typedef struct AuToken
{
    AuTokenType type;
    union
    {
        int whitespace_len;
        AuStringData ident_data;
        AuStringData string_data;
    } data;
} AuToken;

typedef struct AuTokenLiterals
{
    char *literal;
    AuTokenType type;
    bool can_terminate_literal;
} AuTokenMatch;

static const AuTokenMatch token_matches[] = {
        {.literal = AU_TK_NEWLINE, .type = AuTkNewline, .can_terminate_literal = true},
        {.literal = AU_TK_FULLSTOP, .type = AuTkFullStop, .can_terminate_literal = true},
        {.literal = AU_TK_COMMA, .type = AuTkComma, .can_terminate_literal = true},
        {.literal = AU_TK_OPEN_PAREN, .type = AuTkOpenParen, .can_terminate_literal = true},
        {.literal = AU_TK_CLOSE_PAREN, .type = AuTkCloseParen, .can_terminate_literal = true},
        {.literal = AU_TK_ASSIGN, .type = AuTkAssign, .can_terminate_literal = true},
        {.literal = AU_TK_EQUALS, .type = AuTkEquals, .can_terminate_literal = true},
        {.literal = AU_TK_IF, .type = AuTkIf, .can_terminate_literal = false},
        {.literal = AU_TK_THEN, .type = AuTkThen, .can_terminate_literal = false},
        {.literal = AU_TK_ELSE, .type = AuTkElse, .can_terminate_literal = false},
        {.literal = AU_TK_END, .type = AuTkEnd, .can_terminate_literal = false},
        {.literal = AU_TK_USING, .type = AuTkUsing, .can_terminate_literal = false},
        {.literal = AU_TK_DEF, .type = AuTkDef, .can_terminate_literal = false},
        {.literal = AU_TK_NOT, .type = AuTkNot, .can_terminate_literal = false},
        {.literal = AU_TK_AND, .type = AuTkAnd, .can_terminate_literal = false},
        {.literal = AU_TK_OR, .type = AuTkOr, .can_terminate_literal = false},
        {.literal = AU_TK_LIT_NIL, .type = AuTkLitNil, .can_terminate_literal = false},
        {.literal = AU_TK_LIT_TRUE, .type = AuTkLitTrue, .can_terminate_literal = false},
        {.literal = AU_TK_LIT_FALSE, .type = AuTkLitFalse, .can_terminate_literal = false},
};

AuToken *au_build_tokens(const char *, int, int *);

AuStringData make_string_data(const char *, int);

char *au_token_type_to_string(AuTokenType);

#endif
