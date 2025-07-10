#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void parse_error(parser_t *parser, const char *message) {
    exit(0);
}

static void eat(parser_t *parser, token_type_t type) {
    if (parser->current_token.type == type) {
        free(parser->current_token.value);
        parser->current_token = get_next_token(parser->lexer);
    } else {
        char error[256];
        snprintf(error, sizeof(error), "Expected token type %d, got %d",
                 type, parser->current_token.type);
        parse_error(parser, error);
    }
}

static struct type *parse_type(parser_t *parser);
static struct expr *parse_expr(parser_t *parser);
static struct stmt *parse_stmt(parser_t *parser);
static struct decl *parse_decl(parser_t *parser);
static struct decl *parse_comment_decl(parser_t *parser);

parser_t *init_parser(lexer_t *lexer) {
    parser_t *parser = malloc(sizeof(parser_t));
    if (!parser) return NULL;

    parser->lexer = lexer;
    parser->current_token = get_next_token(lexer);
    return parser;
}

void free_parser(parser_t *parser) {
    if (!parser) return;

    free(parser->current_token.value);
    free(parser);
}

static struct decl *parse_comment_decl(parser_t *parser) {
    struct decl *d = NULL;

    if (parser->current_token.type == TOKEN_COMMENT) {
        char *comment_text = strdup(parser->current_token.value);
        eat(parser, TOKEN_COMMENT);
        d = create_comment_decl(comment_text, 0, NULL);
    } else if (parser->current_token.type == TOKEN_MULTI_COMMENT) {
        char *comment_text = strdup(parser->current_token.value);
        eat(parser, TOKEN_MULTI_COMMENT);
        d = create_comment_decl(comment_text, 1, NULL);
    }

    return d;
}

static struct type *parse_type(parser_t *parser) {
    struct type *t = NULL;

    if (parser->current_token.type == TOKEN_ARRAY) {
        eat(parser, TOKEN_ARRAY);

        eat(parser, TOKEN_LBRACKET);
        struct expr *size_expr = parse_expr(parser);
        eat(parser, TOKEN_RBRACKET);

        struct type *element_type = parse_type(parser);

        t = create_type(TYPE_ARRAY, element_type, NULL);
        t->array_size = size_expr;

        return t;
    } else if (parser->current_token.type == TOKEN_FUNCTION) {
        eat(parser, TOKEN_FUNCTION);

        struct type *return_type = parse_type(parser);

        t = create_type(TYPE_FUNCTION, return_type, NULL);

        eat(parser, TOKEN_LPAREN);

        if (parser->current_token.type != TOKEN_RPAREN) {
            struct param_list *params = NULL;
            struct param_list *current = NULL;

            do {
                if (parser->current_token.type != TOKEN_IDENTIFIER) {
                    parse_error(parser, "Expected parameter name");
                }

                char *param_name = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);

                eat(parser, TOKEN_COLON);

                struct type *param_type = parse_type(parser);

                struct param_list *p = create_param(param_name, param_type, NULL);

                if (!params) {
                    params = p;
                    current = p;
                } else {
                    current->next = p;
                    current = p;
                }
            } while (parser->current_token.type == TOKEN_COMMA && (eat(parser, TOKEN_COMMA), 1));

            t->params = params;
        }

        eat(parser, TOKEN_RPAREN);
        return t;
    }

    switch (parser->current_token.type) {
        case TOKEN_VOID:
            eat(parser, TOKEN_VOID);
            t = create_type(TYPE_VOID, NULL, NULL);
            break;
        case TOKEN_INT:
            eat(parser, TOKEN_INT);
            t = create_type(TYPE_INTEGER, NULL, NULL);
            break;
        case TOKEN_BOOLEAN:
            eat(parser, TOKEN_BOOLEAN);
            t = create_type(TYPE_BOOLEAN, NULL, NULL);
            break;
        case TOKEN_CHAR:
            eat(parser, TOKEN_CHAR);
            t = create_type(TYPE_CHARACTER, NULL, NULL);
            break;
        case TOKEN_STRING_TYPE:
            eat(parser, TOKEN_STRING_TYPE);
            t = create_type(TYPE_STRING, NULL, NULL);
            break;
        default:
            parse_error(parser, "Expected type");
    }

    return t;
}

static struct expr *parse_array_initializer(parser_t *parser) {
    eat(parser, TOKEN_LBRACE);

    struct expr *array_expr = NULL;
    struct expr *current = NULL;
    int count = 0;

    if (parser->current_token.type != TOKEN_RBRACE) {
        do {
            struct expr *element = parse_expr(parser);

            if (!element) {
                parse_error(parser, "Expected expression in array initializer");
            }

            count++;

            if (!array_expr) {
                array_expr = element;
                current = element;
            } else {
                current->right = element;
                current = element;
            }
        } while (parser->current_token.type == TOKEN_COMMA && (eat(parser, TOKEN_COMMA), 1));
    }

    eat(parser, TOKEN_RBRACE);

    struct expr *e = create_expr(EXPR_ARRAY_LITERAL, NULL, array_expr);

    if (!e) {
        parse_error(parser, "Failed to create array literal expression");
    }

    if (!e->right && count > 0) {
        parse_error(parser, "Array literal right pointer is NULL but should have elements");
    }

    return e;
}

static struct expr *parse_primary_expr(parser_t *parser) {
    struct expr *e = NULL;

    switch (parser->current_token.type) {
        case TOKEN_INTEGER:
            e = create_expr(EXPR_INTEGER_LITERAL, NULL, NULL);
            e->integer_value = atoi(parser->current_token.value);
            eat(parser, TOKEN_INTEGER);
            break;

        case TOKEN_STRING:
            e = create_expr(EXPR_STRING_LITERAL, NULL, NULL);
            e->string_literal = strdup(parser->current_token.value);
            eat(parser, TOKEN_STRING);
            break;

        case TOKEN_CHARACTER:
            e = create_expr(EXPR_CHAR_LITERAL, NULL, NULL);
            e->integer_value = parser->current_token.value[0];
            eat(parser, TOKEN_CHARACTER);
            break;

        case TOKEN_TRUE:
            e = create_expr(EXPR_BOOL_LITERAL, NULL, NULL);
            e->integer_value = 1;
            eat(parser, TOKEN_TRUE);
            break;

        case TOKEN_FALSE:
            e = create_expr(EXPR_BOOL_LITERAL, NULL, NULL);
            e->integer_value = 0;
            eat(parser, TOKEN_FALSE);
            break;

        case TOKEN_IDENTIFIER:
            e = create_expr(EXPR_NAME, NULL, NULL);
            e->name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            break;

        case TOKEN_LPAREN:
            eat(parser, TOKEN_LPAREN);
            e = parse_expr(parser);
            eat(parser, TOKEN_RPAREN);
            break;

        case TOKEN_LBRACE:
            e = parse_array_initializer(parser);
            break;

        default:
        {
            char error[256];
            snprintf(error, sizeof(error), "Expected expression, got token type %d",
                     parser->current_token.type);
            parse_error(parser, error);
        }
    }

    return e;
}

static struct expr *parse_postfix_expr(parser_t *parser) {
    struct expr *e = parse_primary_expr(parser);

    while (parser->current_token.type == TOKEN_LPAREN || parser->current_token.type == TOKEN_LBRACKET) {
        if (parser->current_token.type == TOKEN_LPAREN) {
            eat(parser, TOKEN_LPAREN);

            struct expr *args = NULL;
            struct expr *current = NULL;

            if (parser->current_token.type != TOKEN_RPAREN) {
                do {
                    struct expr *arg = parse_expr(parser);

                    if (!args) {
                        args = arg;
                        current = arg;
                    } else {
                        current->right = arg;
                        current = arg;
                    }
                } while (parser->current_token.type == TOKEN_COMMA && (eat(parser, TOKEN_COMMA), 1));
            }

            eat(parser, TOKEN_RPAREN);

            e = create_expr(EXPR_CALL, e, args);
        } else if (parser->current_token.type == TOKEN_LBRACKET) {
            eat(parser, TOKEN_LBRACKET);

            struct expr *index = parse_expr(parser);

            eat(parser, TOKEN_RBRACKET);

            e = create_expr(EXPR_SUBSCRIPT, e, index);
        }
    }

    return e;
}

static struct expr *parse_unary_expr(parser_t *parser) {
    struct expr *e = NULL;

    if (parser->current_token.type == TOKEN_MINUS) {
        eat(parser, TOKEN_MINUS);
        e = create_expr(EXPR_UNARY_MINUS, NULL, parse_unary_expr(parser));
    } else if (parser->current_token.type == TOKEN_NOT) {
        eat(parser, TOKEN_NOT);
        e = create_expr(EXPR_NOT, NULL, parse_unary_expr(parser));
    } else {
        e = parse_postfix_expr(parser);
    }

    return e;
}

static struct expr *parse_power_expr(parser_t *parser) {
    struct expr *e = parse_unary_expr(parser);

    if (parser->current_token.type == TOKEN_CARET) {
        eat(parser, TOKEN_CARET);
        e = create_expr(EXPR_POWER, e, parse_power_expr(parser));
    }

    return e;
}

static struct expr *parse_multiplicative_expr(parser_t *parser) {
    struct expr *e = parse_power_expr(parser);

    while (parser->current_token.type == TOKEN_STAR ||
           parser->current_token.type == TOKEN_SLASH ||
           parser->current_token.type == TOKEN_PERCENT) {
        token_type_t op = parser->current_token.type;
        eat(parser, op);

        struct expr *right = parse_power_expr(parser);

        switch (op) {
            case TOKEN_STAR:
                e = create_expr(EXPR_MUL, e, right);
                break;
            case TOKEN_SLASH:
                e = create_expr(EXPR_DIV, e, right);
                break;
            case TOKEN_PERCENT:
                e = create_expr(EXPR_MOD, e, right);
                break;
            default:
                break;
        }
    }

    return e;
}

static struct expr *parse_additive_expr(parser_t *parser) {
    struct expr *e = parse_multiplicative_expr(parser);

    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS) {
        token_type_t op = parser->current_token.type;
        eat(parser, op);

        struct expr *right = parse_multiplicative_expr(parser);

        switch (op) {
            case TOKEN_PLUS:
                e = create_expr(EXPR_ADD, e, right);
                break;
            case TOKEN_MINUS:
                e = create_expr(EXPR_SUB, e, right);
                break;
            default:
                break;
        }
    }

    return e;
}

static struct expr *parse_relational_expr(parser_t *parser) {
    struct expr *e = parse_additive_expr(parser);

    while (parser->current_token.type == TOKEN_LT ||
           parser->current_token.type == TOKEN_GT ||
           parser->current_token.type == TOKEN_LE ||
           parser->current_token.type == TOKEN_GE) {
        token_type_t op = parser->current_token.type;
        eat(parser, op);

        struct expr *right = parse_additive_expr(parser);

        switch (op) {
            case TOKEN_LT:
                e = create_expr(EXPR_LT, e, right);
                break;
            case TOKEN_GT:
                e = create_expr(EXPR_GT, e, right);
                break;
            case TOKEN_LE:
                e = create_expr(EXPR_LE, e, right);
                break;
            case TOKEN_GE:
                e = create_expr(EXPR_GE, e, right);
                break;
            default:
                break;
        }
    }

    return e;
}

static struct expr *parse_equality_expr(parser_t *parser) {
    struct expr *e = parse_relational_expr(parser);

    while (parser->current_token.type == TOKEN_EQ ||
           parser->current_token.type == TOKEN_NEQ) {
        token_type_t op = parser->current_token.type;
        eat(parser, op);

        struct expr *right = parse_relational_expr(parser);

        switch (op) {
            case TOKEN_EQ:
                e = create_expr(EXPR_EQ, e, right);
                break;
            case TOKEN_NEQ:
                e = create_expr(EXPR_NEQ, e, right);
                break;
            default:
                break;
        }
    }

    return e;
}

static struct expr *parse_logical_and_expr(parser_t *parser) {
    struct expr *e = parse_equality_expr(parser);

    while (parser->current_token.type == TOKEN_AND) {
        eat(parser, TOKEN_AND);

        struct expr *right = parse_equality_expr(parser);
        e = create_expr(EXPR_AND, e, right);
    }

    return e;
}

static struct expr *parse_logical_or_expr(parser_t *parser) {
    struct expr *e = parse_logical_and_expr(parser);

    while (parser->current_token.type == TOKEN_OR) {
        eat(parser, TOKEN_OR);

        struct expr *right = parse_logical_and_expr(parser);
        e = create_expr(EXPR_OR, e, right);
    }

    return e;
}

static struct expr *parse_assignment_expr(parser_t *parser) {
    struct expr *e = parse_logical_or_expr(parser);

    if (parser->current_token.type == TOKEN_ASSIGN) {
        eat(parser, TOKEN_ASSIGN);

        struct expr *right = parse_assignment_expr(parser);
        e = create_expr(EXPR_ASSIGN, e, right);
    }

    return e;
}

static struct expr *parse_expr(parser_t *parser) {
    return parse_assignment_expr(parser);
}

static struct stmt *parse_comment(parser_t *parser) {
    struct stmt *s = create_stmt(STMT_COMMENT);
    s->comment_text = strdup(parser->current_token.value);

    eat(parser, TOKEN_COMMENT);

    return s;
}

static struct stmt *parse_multi_comment(parser_t *parser) {
    struct stmt *s = create_stmt(STMT_MULTI_COMMENT);
    s->comment_text = strdup(parser->current_token.value);

    eat(parser, TOKEN_MULTI_COMMENT);

    return s;
}

static struct expr *parse_print_args(parser_t *parser) {
    struct expr *first_expr = parse_expr(parser);
    if (!first_expr) {
        return NULL;
    }

    struct expr *arg_list = create_expr(EXPR_ARG, first_expr, NULL);
    struct expr *current = arg_list;

    while (parser->current_token.type == TOKEN_COMMA) {
        eat(parser, TOKEN_COMMA);

        struct expr *next_expr = parse_expr(parser);
        if (!next_expr) {
            continue;
        }

        struct expr *next_arg = create_expr(EXPR_ARG, next_expr, NULL);

        current->right = next_arg;
        current = next_arg;
    }

    return arg_list;
}

static struct stmt *parse_stmt(parser_t *parser) {
    struct stmt *s = NULL;

    switch (parser->current_token.type) {
        case TOKEN_IDENTIFIER:
            if (parser->lexer->position < parser->lexer->length &&
                parser->lexer->input[parser->lexer->position] == ':') {
                s = create_stmt(STMT_DECL);
                s->decl = parse_decl(parser);
            } else {
                s = create_stmt(STMT_EXPR);
                s->expr = parse_expr(parser);
                eat(parser, TOKEN_SEMICOLON);
            }
            break;

        case TOKEN_FUNCTION:
            s = create_stmt(STMT_DECL);
            s->decl = parse_decl(parser);
            break;

        case TOKEN_IF:
            s = create_stmt(STMT_IF_ELSE);
            eat(parser, TOKEN_IF);
            eat(parser, TOKEN_LPAREN);
            s->expr = parse_expr(parser);
            eat(parser, TOKEN_RPAREN);
            s->body = parse_stmt(parser);

            if (parser->current_token.type == TOKEN_ELSE) {
                eat(parser, TOKEN_ELSE);
                s->else_body = parse_stmt(parser);
            }
            break;

        case TOKEN_FOR:
            s = create_stmt(STMT_FOR);
            eat(parser, TOKEN_FOR);
            eat(parser, TOKEN_LPAREN);

            if (parser->current_token.type != TOKEN_SEMICOLON) {
                s->init_expr = parse_expr(parser);
            }
            eat(parser, TOKEN_SEMICOLON);

            if (parser->current_token.type != TOKEN_SEMICOLON) {
                s->expr = parse_expr(parser);
            }
            eat(parser, TOKEN_SEMICOLON);

            if (parser->current_token.type != TOKEN_RPAREN) {
                s->next_expr = parse_expr(parser);
            }
            eat(parser, TOKEN_RPAREN);

            s->body = parse_stmt(parser);
            break;

        case TOKEN_RETURN:
            s = create_stmt(STMT_RETURN);
            eat(parser, TOKEN_RETURN);

            if (parser->current_token.type != TOKEN_SEMICOLON) {
                s->expr = parse_expr(parser);
            }
            eat(parser, TOKEN_SEMICOLON);
            break;

        case TOKEN_PRINT:
            s = create_stmt(STMT_PRINT);
            eat(parser, TOKEN_PRINT);

            s->expr = parse_print_args(parser);

            eat(parser, TOKEN_SEMICOLON);
            break;

        case TOKEN_LBRACE:
            s = create_stmt(STMT_BLOCK);
            eat(parser, TOKEN_LBRACE);

            struct stmt *block_stmt = NULL;
            struct stmt *current = NULL;

            while (parser->current_token.type != TOKEN_RBRACE &&
                   parser->current_token.type != TOKEN_EOF) {
                struct stmt *stmt = parse_stmt(parser);

                if (!block_stmt) {
                    block_stmt = stmt;
                    current = stmt;
                } else {
                    current->next = stmt;
                    current = stmt;
                }
            }

            eat(parser, TOKEN_RBRACE);
            s->body = block_stmt;
            break;

        case TOKEN_COMMENT:
            s = parse_comment(parser);
            break;

        case TOKEN_MULTI_COMMENT:
            s = parse_multi_comment(parser);
            break;

        default:
            s = create_stmt(STMT_EXPR);
            s->expr = parse_expr(parser);
            eat(parser, TOKEN_SEMICOLON);
            break;
    }

    return s;
}

static struct decl *parse_decl(parser_t *parser) {
    if (parser->current_token.type == TOKEN_COMMENT ||
        parser->current_token.type == TOKEN_MULTI_COMMENT) {
        return parse_comment_decl(parser);
    }

    char *name = NULL;
    struct type *t = NULL;
    struct decl *d = NULL;

    if (parser->current_token.type == TOKEN_FUNCTION) {
        eat(parser, TOKEN_FUNCTION);

        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parse_error(parser, "Expected function name");
        }

        name = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);

        t = create_type(TYPE_FUNCTION, NULL, NULL);

        eat(parser, TOKEN_LPAREN);

        if (parser->current_token.type != TOKEN_RPAREN) {
            struct param_list *params = NULL;
            struct param_list *current = NULL;

            do {
                struct type *param_type = parse_type(parser);

                if (parser->current_token.type != TOKEN_IDENTIFIER) {
                    parse_error(parser, "Expected parameter name");
                }

                char *param_name = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);

                struct param_list *p = create_param(param_name, param_type, NULL);

                if (!params) {
                    params = p;
                    current = p;
                } else {
                    current->next = p;
                    current = p;
                }
            } while (parser->current_token.type == TOKEN_COMMA && (eat(parser, TOKEN_COMMA), 1));

            t->params = params;
        }

        eat(parser, TOKEN_RPAREN);
        eat(parser, TOKEN_COLON);

        struct type *return_type = parse_type(parser);
        t->subtype = return_type;

        d = create_decl(name, t, NULL, NULL, NULL);
        if (d) {
            d->kind = DECL_FUNCTION;
            d->code = parse_stmt(parser);
        }

        return d;
    } else {
        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parse_error(parser, "Expected identifier for declaration");
        }

        name = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);

        if (parser->current_token.type != TOKEN_COLON) {
            parse_error(parser, "Expected colon after identifier in declaration");
        }
        eat(parser, TOKEN_COLON);

        t = parse_type(parser);

        d = create_decl(name, t, NULL, NULL, NULL);
        if (d) {
            d->kind = DECL_VARIABLE;

            if (parser->current_token.type == TOKEN_ASSIGN) {
                eat(parser, TOKEN_ASSIGN);

                if (t->kind == TYPE_FUNCTION && parser->current_token.type == TOKEN_LBRACE) {
                    d->code = parse_stmt(parser);
                } else {
                    if (t->kind == TYPE_ARRAY && parser->current_token.type == TOKEN_LBRACE) {
                        d->value = parse_array_initializer(parser);

                        if (!d->value) {
                            parse_error(parser, "Failed to create array initializer");
                        }

                        if (!d->value->right && d->value->kind == EXPR_ARRAY_LITERAL) {
                            parse_error(parser, "Array initializer has NULL right pointer");
                        }
                    } else {
                        d->value = parse_expr(parser);
                    }

                    if (t->kind != TYPE_FUNCTION) {
                        eat(parser, TOKEN_SEMICOLON);
                    }
                }
            } else if (parser->current_token.type == TOKEN_SEMICOLON) {
                eat(parser, TOKEN_SEMICOLON);
            }
        }

        return d;
    }
}

struct decl *parse_program(parser_t *parser) {
    if (!parser) return NULL;

    struct decl *program = NULL;
    struct decl *current = NULL;

    while (parser->current_token.type != TOKEN_EOF) {
        struct decl *d = NULL;

        if (parser->current_token.type == TOKEN_COMMENT ||
            parser->current_token.type == TOKEN_MULTI_COMMENT) {
            d = parse_comment_decl(parser);
        } else {
            d = parse_decl(parser);
        }

        if (d) {
            if (!program) {
                program = d;
                current = d;
            } else {
                current->next = d;
                current = d;
            }
        }
    }

    return program;
}