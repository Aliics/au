#ifndef AU_TOKEN_H
#define AU_TOKEN_H

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
} AUStringData;

typedef struct AuToken
{
    AuTokenType type;
    union
    {
        int whitespace_len;
        AUStringData ident_data;
        AUStringData string_data;
    } data;
} AuToken;

typedef struct AuTokenLiterals
{
    char *literal;
    AuTokenType type;
} AuTokenMatch;

static const AuTokenMatch token_literals[] = {
        {.literal = AU_TK_NEWLINE, .type = AuTkNewline},
        {.literal = AU_TK_FULLSTOP, .type = AuTkFullStop},
        {.literal = AU_TK_COMMA, .type = AuTkComma},
        {.literal = AU_TK_OPEN_PAREN, .type = AuTkOpenParen},
        {.literal = AU_TK_CLOSE_PAREN, .type = AuTkCloseParen},
        {.literal = AU_TK_ASSIGN, .type = AuTkAssign},
        {.literal = AU_TK_EQUALS, .type = AuTkEquals},
        {.literal = AU_TK_IF, .type = AuTkIf},
        {.literal = AU_TK_THEN, .type = AuTkThen},
        {.literal = AU_TK_ELSE, .type = AuTkElse},
        {.literal = AU_TK_END, .type = AuTkEnd},
        {.literal = AU_TK_USING, .type = AuTkUsing},
        {.literal = AU_TK_DEF, .type = AuTkDef},
        {.literal = AU_TK_AND, .type = AuTkAnd},
        {.literal = AU_TK_OR, .type = AuTkOr},
        {.literal = AU_TK_LIT_NIL, .type = AuTkLitNil},
        {.literal = AU_TK_LIT_TRUE, .type = AuTkLitTrue},
        {.literal = AU_TK_LIT_FALSE, .type = AuTkLitFalse},
};

AuToken *au_build_tokens(const char *, int, int *);

AUStringData make_string_data(const char *, int);

char *au_token_type_string(AuTokenType);

#endif
