#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef enum {
    TYPE_VOID,
    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_FUNCTION
} type_kind_t;

typedef enum {
    EXPR_NAME,
    EXPR_INTEGER_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_CHAR_LITERAL,
    EXPR_BOOL_LITERAL,
    EXPR_CALL,
    EXPR_UNARY_MINUS,
    EXPR_NOT,
    EXPR_POWER,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_MOD,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_LT,
    EXPR_LE,
    EXPR_GT,
    EXPR_GE,
    EXPR_EQ,
    EXPR_NEQ,
    EXPR_AND,
    EXPR_OR,
    EXPR_ASSIGN,
    EXPR_SUBSCRIPT,
    EXPR_ARRAY_LITERAL,
    EXPR_ARG
} expr_kind_t;

typedef enum {
    STMT_DECL,
    STMT_EXPR,
    STMT_IF_ELSE,
    STMT_FOR,
    STMT_PRINT,
    STMT_RETURN,
    STMT_BLOCK,
    STMT_COMMENT,
    STMT_MULTI_COMMENT
} stmt_kind_t;

typedef enum {
    DECL_VARIABLE,
    DECL_FUNCTION,
    DECL_COMMENT,
    DECL_MULTI_COMMENT
} decl_kind_t;

struct type {
    type_kind_t kind;
    struct type *subtype;
    struct param_list *params;
    struct expr *array_size;
};

struct param_list {
    char *name;
    struct type *type;
    struct param_list *next;
};

struct expr {
    expr_kind_t kind;
    struct expr *left;
    struct expr *right;

    char *name;
    int integer_value;
    char *string_literal;
};

struct stmt {
    stmt_kind_t kind;
    struct expr *expr;
    struct expr *init_expr;
    struct expr *next_expr;
    struct stmt *body;
    struct stmt *else_body;
    struct stmt *next;
    struct decl *decl;
    char *comment_text;
};

struct decl {
    char *name;
    struct type *type;
    struct expr *value;
    struct stmt *code;
    struct decl *next;
    decl_kind_t kind;
    char *comment_text;
};

struct type *create_type(type_kind_t kind, struct type *subtype, struct param_list *params);
struct param_list *create_param(char *name, struct type *type, struct param_list *next);
struct expr *create_expr(expr_kind_t kind, struct expr *left, struct expr *right);
struct stmt *create_stmt(stmt_kind_t kind);
struct decl *create_decl(char *name, struct type *type, struct expr *value, struct stmt *code, struct decl *next);
struct decl *create_comment_decl(char *comment_text, int is_multi, struct decl *next);

void print_type(struct type *t);
void print_expr(struct expr *e);
void print_stmt(struct stmt *s, int indent);
void print_decl(struct decl *d, int indent);

#endif