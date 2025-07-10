#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// Parser structure
typedef struct {
    lexer_t *lexer;
    token_t current_token;
} parser_t;

// Parser functions
parser_t *init_parser(lexer_t *lexer);
struct decl *parse_program(parser_t *parser);
void free_parser(parser_t *parser);

#endif /* PARSER_H */