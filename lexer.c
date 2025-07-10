#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

lexer_t *init_lexer(char *input) {
    lexer_t *lexer = malloc(sizeof(lexer_t));
    if (!lexer) return NULL;

    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->length = strlen(input);

    return lexer;
}

void free_lexer(lexer_t *lexer) {
    if (lexer) {
        free(lexer);
    }
}

static char peek(lexer_t *lexer) {
    if (lexer->position >= lexer->length) {
        return '\0';
    }
    return lexer->input[lexer->position];
}

static char peek_next(lexer_t *lexer) {
    if (lexer->position + 1 >= lexer->length) {
        return '\0';
    }
    return lexer->input[lexer->position + 1];
}

static char advance(lexer_t *lexer) {
    char c = peek(lexer);
    lexer->position++;
    lexer->column++;

    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    }

    return c;
}

static void skip_whitespace(lexer_t *lexer) {
    while (isspace(peek(lexer))) {
        advance(lexer);
    }
}

static token_t create_token(token_type_t type, char *value, int line, int column) {
    token_t token;
    token.type = type;
    token.value = value;
    token.line = line;
    token.column = column;
    return token;
}

static int is_keyword(const char *str) {
    if (strcmp(str, "if") == 0) return TOKEN_IF;
    if (strcmp(str, "else") == 0) return TOKEN_ELSE;
    if (strcmp(str, "for") == 0) return TOKEN_FOR;
    if (strcmp(str, "return") == 0) return TOKEN_RETURN;
    if (strcmp(str, "print") == 0) return TOKEN_PRINT;
    if (strcmp(str, "void") == 0) return TOKEN_VOID;
    if (strcmp(str, "boolean") == 0) return TOKEN_BOOLEAN;
    if (strcmp(str, "char") == 0) return TOKEN_CHAR;
    if (strcmp(str, "integer") == 0) return TOKEN_INT;
    if (strcmp(str, "string") == 0) return TOKEN_STRING_TYPE;
    if (strcmp(str, "array") == 0) return TOKEN_ARRAY;
    if (strcmp(str, "function") == 0) return TOKEN_FUNCTION;
    if (strcmp(str, "true") == 0) return TOKEN_TRUE;
    if (strcmp(str, "false") == 0) return TOKEN_FALSE;
    return 0;
}

static token_t read_identifier(lexer_t *lexer) {
    int start_pos = lexer->position;
    int start_column = lexer->column;
    int line = lexer->line;

    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }

    int length = lexer->position - start_pos;
    char *value = malloc(length + 1);
    strncpy(value, lexer->input + start_pos, length);
    value[length] = '\0';

    int keyword_type = is_keyword(value);
    if (keyword_type) {
        free(value);
        return create_token(keyword_type, NULL, line, start_column);
    }

    return create_token(TOKEN_IDENTIFIER, value, line, start_column);
}

static token_t read_number(lexer_t *lexer) {
    int start_pos = lexer->position;
    int start_column = lexer->column;
    int line = lexer->line;

    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    int length = lexer->position - start_pos;
    char *value = malloc(length + 1);
    strncpy(value, lexer->input + start_pos, length);
    value[length] = '\0';

    return create_token(TOKEN_INTEGER, value, line, start_column);
}

static token_t read_string(lexer_t *lexer) {
    int start_pos = lexer->position;
    int start_column = lexer->column;
    int line = lexer->line;

    advance(lexer);

    while (peek(lexer) != '"' && peek(lexer) != '\0') {
        if (peek(lexer) == '\\' && peek_next(lexer) == '"') {
            advance(lexer);
        }
        advance(lexer);
    }

    if (peek(lexer) == '\0') {
        return create_token(TOKEN_EOF, NULL, line, start_column);
    }

    advance(lexer);

    int length = lexer->position - start_pos - 2;
    char *value = malloc(length + 1);
    strncpy(value, lexer->input + start_pos + 1, length);
    value[length] = '\0';

    return create_token(TOKEN_STRING, value, line, start_column);
}

static token_t read_character(lexer_t *lexer) {
    int start_pos = lexer->position;
    int start_column = lexer->column;
    int line = lexer->line;

    advance(lexer);

    if (peek(lexer) == '\\') {
        advance(lexer);
        if (peek(lexer) == 'n' || peek(lexer) == 't' || peek(lexer) == '\\' || peek(lexer) == '\'') {
            advance(lexer);
        }
    } else if (peek(lexer) != '\'') {
        advance(lexer);
    }

    if (peek(lexer) != '\'') {
        return create_token(TOKEN_EOF, NULL, line, start_column);
    }

    advance(lexer);

    int length = lexer->position - start_pos - 2;
    char *value = malloc(length + 1);
    strncpy(value, lexer->input + start_pos + 1, length);
    value[length] = '\0';

    return create_token(TOKEN_CHARACTER, value, line, start_column);
}

static token_t read_comment(lexer_t *lexer) {
    int start_pos = lexer->position;
    int start_column = lexer->column;
    int line = lexer->line;

    // Skip the first two characters: '/' and either '/' or '*'
    advance(lexer);
    advance(lexer);

    if (lexer->input[start_pos + 1] == '*') {
        // Multi-line comment
        // Continue until we find closing */
        while (!(peek(lexer) == '*' && peek_next(lexer) == '/')) {
            if (peek(lexer) == '\0') {
                // Reached end of file without closing comment
                return create_token(TOKEN_EOF, NULL, line, start_column);
            }
            advance(lexer);
        }

        // Skip the closing */
        advance(lexer); // Skip *
        advance(lexer); // Skip /

        // Calculate the length and create token
        int length = lexer->position - start_pos;
        char *value = malloc(length + 1);
        strncpy(value, lexer->input + start_pos, length);
        value[length] = '\0';

        return create_token(TOKEN_MULTI_COMMENT, value, line, start_column);
    } else {
        // Single-line comment
        while (peek(lexer) != '\n' && peek(lexer) != '\0') {
            advance(lexer);
        }

        int length = lexer->position - start_pos;
        char *value = malloc(length + 1);
        strncpy(value, lexer->input + start_pos, length);
        value[length] = '\0';

        return create_token(TOKEN_COMMENT, value, line, start_column);
    }
}

token_t get_next_token(lexer_t *lexer) {
    skip_whitespace(lexer);

    char c = peek(lexer);
    int line = lexer->line;
    int column = lexer->column;

    if (c == '\0') {
        return create_token(TOKEN_EOF, NULL, line, column);
    }

    if (isalpha(c) || c == '_') {
        return read_identifier(lexer);
    }

    if (isdigit(c)) {
        return read_number(lexer);
    }

    if (c == '"') {
        return read_string(lexer);
    }

    if (c == '\'') {
        return read_character(lexer);
    }

    if (c == '/' && (peek_next(lexer) == '/' || peek_next(lexer) == '*')) {
        return read_comment(lexer);
    }

    advance(lexer);

    switch (c) {
        case '+': return create_token(TOKEN_PLUS, NULL, line, column);
        case '-': return create_token(TOKEN_MINUS, NULL, line, column);
        case '*': return create_token(TOKEN_STAR, NULL, line, column);
        case '/': return create_token(TOKEN_SLASH, NULL, line, column);
        case '%': return create_token(TOKEN_PERCENT, NULL, line, column);
        case '^': return create_token(TOKEN_CARET, NULL, line, column);
        case '(': return create_token(TOKEN_LPAREN, NULL, line, column);
        case ')': return create_token(TOKEN_RPAREN, NULL, line, column);
        case '{': return create_token(TOKEN_LBRACE, NULL, line, column);
        case '}': return create_token(TOKEN_RBRACE, NULL, line, column);
        case '[': return create_token(TOKEN_LBRACKET, NULL, line, column);
        case ']': return create_token(TOKEN_RBRACKET, NULL, line, column);
        case ';': return create_token(TOKEN_SEMICOLON, NULL, line, column);
        case ':': return create_token(TOKEN_COLON, NULL, line, column);
        case ',': return create_token(TOKEN_COMMA, NULL, line, column);

        case '=':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_EQ, NULL, line, column);
            }
            return create_token(TOKEN_ASSIGN, NULL, line, column);

        case '!':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_NEQ, NULL, line, column);
            }
            return create_token(TOKEN_NOT, NULL, line, column);

        case '<':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_LE, NULL, line, column);
            }
            return create_token(TOKEN_LT, NULL, line, column);

        case '>':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_GE, NULL, line, column);
            }
            return create_token(TOKEN_GT, NULL, line, column);

        case '&':
            if (peek(lexer) == '&') {
                advance(lexer);
                return create_token(TOKEN_AND, NULL, line, column);
            }
            break;

        case '|':
            if (peek(lexer) == '|') {
                advance(lexer);
                return create_token(TOKEN_OR, NULL, line, column);
            }
            break;
    }

    char unknown[2] = {c, '\0'};
    char *value = strdup(unknown);
    return create_token(TOKEN_EOF, value, line, column);
}