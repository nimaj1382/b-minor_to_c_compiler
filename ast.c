#include <stdlib.h>
#include <string.h>
#include "ast.h"

struct type *create_type(type_kind_t kind, struct type *subtype, struct param_list *params) {
    struct type *t = malloc(sizeof(struct type));
    if (!t) return NULL;

    t->kind = kind;
    t->subtype = subtype;
    t->params = params;
    t->array_size = NULL;

    return t;
}

struct param_list *create_param(char *name, struct type *type, struct param_list *next) {
    struct param_list *p = malloc(sizeof(struct param_list));
    if (!p) return NULL;

    p->name = name;
    p->type = type;
    p->next = next;

    return p;
}

struct expr *create_expr(expr_kind_t kind, struct expr *left, struct expr *right) {
    struct expr *e = malloc(sizeof(struct expr));
    if (!e) return NULL;

    e->kind = kind;
    e->left = left;
    e->right = right;
    e->name = NULL;
    e->integer_value = 0;
    e->string_literal = NULL;

    return e;
}

struct stmt *create_stmt(stmt_kind_t kind) {
    struct stmt *s = malloc(sizeof(struct stmt));
    if (!s) return NULL;

    s->kind = kind;
    s->expr = NULL;
    s->init_expr = NULL;
    s->next_expr = NULL;
    s->body = NULL;
    s->else_body = NULL;
    s->next = NULL;
    s->decl = NULL;
    s->comment_text = NULL;

    return s;
}

struct decl *create_decl(char *name, struct type *type, struct expr *value, struct stmt *code, struct decl *next) {
    struct decl *d = malloc(sizeof(struct decl));
    if (!d) return NULL;

    d->name = name;
    d->type = type;
    d->value = value;
    d->code = code;
    d->next = next;
    d->kind = type && type->kind == TYPE_FUNCTION ? DECL_FUNCTION : DECL_VARIABLE;
    d->comment_text = NULL;

    return d;
}

struct decl *create_comment_decl(char *comment_text, int is_multi, struct decl *next) {
    struct decl *d = malloc(sizeof(struct decl));
    if (!d) return NULL;

    d->name = NULL;
    d->type = NULL;
    d->value = NULL;
    d->code = NULL;
    d->next = next;
    d->kind = is_multi ? DECL_MULTI_COMMENT : DECL_COMMENT;
    d->comment_text = comment_text;

    return d;
}

void print_type(struct type *t) {
    if (!t) return;

    switch (t->kind) {
        case TYPE_VOID:
            printf("void");
            break;
        case TYPE_BOOLEAN:
            printf("boolean");
            break;
        case TYPE_CHARACTER:
            printf("char");
            break;
        case TYPE_INTEGER:
            printf("integer");
            break;
        case TYPE_STRING:
            printf("string");
            break;
        case TYPE_ARRAY:
            printf("array [");
            if (t->array_size) {
                print_expr(t->array_size);
            }
            printf("] ");
            print_type(t->subtype);
            break;
        case TYPE_FUNCTION:
            printf("function ");
            print_type(t->subtype);
            printf(" (");

            struct param_list *p = t->params;
            while (p) {
                print_type(p->type);
                printf(" %s", p->name);
                p = p->next;
                if (p) printf(", ");
            }

            printf(")");
            break;
    }
}

void print_expr(struct expr *e) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_NAME:
            printf("%s", e->name);
            break;
        case EXPR_INTEGER_LITERAL:
            printf("%d", e->integer_value);
            break;
        case EXPR_STRING_LITERAL:
            printf("\"%s\"", e->string_literal);
            break;
        case EXPR_CHAR_LITERAL:
            printf("'%c'", e->integer_value);
            break;
        case EXPR_BOOL_LITERAL:
            printf("%s", e->integer_value ? "true" : "false");
            break;
        case EXPR_CALL:
            print_expr(e->left);
            printf("(");
            print_expr(e->right);
            printf(")");
            break;
        default:
            printf("(");
            print_expr(e->left);

            switch (e->kind) {
                case EXPR_ADD: printf(" + "); break;
                case EXPR_SUB: printf(" - "); break;
                case EXPR_MUL: printf(" * "); break;
                case EXPR_DIV: printf(" / "); break;
                case EXPR_MOD: printf(" %% "); break;
                case EXPR_EQ: printf(" == "); break;
                case EXPR_NEQ: printf(" != "); break;
                case EXPR_LT: printf(" < "); break;
                case EXPR_GT: printf(" > "); break;
                case EXPR_LE: printf(" <= "); break;
                case EXPR_GE: printf(" >= "); break;
                case EXPR_AND: printf(" && "); break;
                case EXPR_OR: printf(" || "); break;
                default: printf(" ? "); break;
            }

            print_expr(e->right);
            printf(")");
            break;
    }
}

void print_stmt(struct stmt *s, int indent) {
    if (!s) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (s->kind) {
        case STMT_DECL:
            print_decl(s->decl, 0);
            break;
        case STMT_EXPR:
            print_expr(s->expr);
            printf(";\n");
            break;
        case STMT_IF_ELSE:
            printf("if (");
            print_expr(s->expr);
            printf(") {\n");
            print_stmt(s->body, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("}\n");

            if (s->else_body) {
                for (int i = 0; i < indent; i++) {
                    printf("  ");
                }
                printf("else {\n");
                print_stmt(s->else_body, indent + 1);
                for (int i = 0; i < indent; i++) {
                    printf("  ");
                }
                printf("}\n");
            }
            break;
        case STMT_FOR:
            printf("for (");
            if (s->init_expr) {
                print_expr(s->init_expr);
            }
            printf("; ");
            if (s->expr) {
                print_expr(s->expr);
            }
            printf("; ");
            if (s->next_expr) {
                print_expr(s->next_expr);
            }
            printf(") {\n");
            print_stmt(s->body, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("}\n");
            break;
        case STMT_PRINT:
            printf("print ");
            print_expr(s->expr);
            printf(";\n");
            break;
        case STMT_RETURN:
            printf("return");
            if (s->expr) {
                printf(" ");
                print_expr(s->expr);
            }
            printf(";\n");
            break;
        case STMT_BLOCK:
            printf("{\n");
            print_stmt(s->body, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("}\n");
            break;
        case STMT_COMMENT:
            printf("%s\n", s->comment_text);
            break;
        case STMT_MULTI_COMMENT:
            printf("%s\n", s->comment_text);
            break;
    }

    if (s->next) {
        print_stmt(s->next, indent);
    }
}

void print_decl(struct decl *d, int indent) {
    if (!d) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (d->kind) {
        case DECL_VARIABLE:
        case DECL_FUNCTION:
            printf("%s: ", d->name);
            print_type(d->type);

            if (d->value) {
                printf(" = ");
                print_expr(d->value);
            }

            if (d->code) {
                printf(" = {\n");
                print_stmt(d->code, indent + 1);
                for (int i = 0; i < indent; i++) {
                    printf("  ");
                }
                printf("}\n");
            } else {
                printf(";\n");
            }
            break;

        case DECL_COMMENT:
            printf("%s\n", d->comment_text);
            break;

        case DECL_MULTI_COMMENT:
            printf("%s\n", d->comment_text);
            break;
    }

    if (d->next) {
        print_decl(d->next, indent);
    }
}