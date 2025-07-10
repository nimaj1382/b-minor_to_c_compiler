#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_VOID,
    TOKEN_BOOLEAN,
    TOKEN_CHAR,
    TOKEN_INT,
    TOKEN_STRING_TYPE,
    TOKEN_ARRAY,
    TOKEN_FUNCTION,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_COMMENT,
    TOKEN_MULTI_COMMENT
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;
    int line;
    int column;
} token_t;

typedef struct {
    char *input;
    int position;
    int line;
    int column;
    int length;
} lexer_t;

lexer_t *init_lexer(char *input);
void free_lexer(lexer_t *lexer);
token_t get_next_token(lexer_t *lexer);

#endif