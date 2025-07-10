#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

typedef struct symbol_entry {
    char *name;
    type_kind_t type;
    struct symbol_entry *next;
} symbol_entry_t;

typedef struct symbol_table {
    symbol_entry_t *head;
} symbol_table_t;

static symbol_table_t *global_symbols = NULL;

static symbol_table_t *create_symbol_table() {
    symbol_table_t *table = malloc(sizeof(symbol_table_t));
    if (table) {
        table->head = NULL;
    }
    return table;
}

static void add_symbol(symbol_table_t *table, const char *name, type_kind_t type) {
    if (!table) return;

    symbol_entry_t *current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            current->type = type;
            return;
        }
        current = current->next;
    }

    symbol_entry_t *entry = malloc(sizeof(symbol_entry_t));
    if (entry) {
        entry->name = strdup(name);
        entry->type = type;
        entry->next = table->head;
        table->head = entry;
    }
}

static type_kind_t lookup_symbol(symbol_table_t *table, const char *name) {
    if (!table) return TYPE_INTEGER;

    symbol_entry_t *current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->type;
        }
        current = current->next;
    }

    return TYPE_INTEGER;
}

static void free_symbol_table(symbol_table_t *table) {
    if (!table) return;

    symbol_entry_t *current = table->head;
    while (current) {
        symbol_entry_t *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }

    free(table);
}

static void generate_expr_c(struct expr *e, FILE *output);

static void process_string_for_c(const char *str, FILE *output) {
    if (!str) return;

    while (*str) {
        if (*str == '\n') {
            fprintf(output, "\\n");
        } else if (*str == '\\' && *(str+1) == 'n') {
            fprintf(output, "\\n");
            str++;
        } else if (*str == '"') {
            fprintf(output, "\\\"");
        } else if (*str == '\\') {
            fprintf(output, "\\");
            if (*(str+1)) {
                fprintf(output, "%c", *(str+1));
                str++;
            }
        } else {
            fprintf(output, "%c", *str);
        }
        str++;
    }
}

static void generate_comment(const char *comment_text, FILE *output, int is_multi) {
    if (!comment_text) return;

    if (is_multi) {
        fprintf(output, "/*\n");

        char comment_copy[4096];
        strncpy(comment_copy, comment_text + 2, sizeof(comment_copy) - 1);
        comment_copy[sizeof(comment_copy) - 1] = '\0';

        int len = strlen(comment_copy);
        if (len >= 2 && comment_copy[len-2] == '*' && comment_copy[len-1] == '/') {
            comment_copy[len-2] = '\0';
        }

        char *saveptr;
        char *line = strtok_r(comment_copy, "\n", &saveptr);
        while (line) {
            fprintf(output, " * %s\n", line);
            line = strtok_r(NULL, "\n", &saveptr);
        }

        fprintf(output, " */\n");
    } else {
        fprintf(output, "%s\n", comment_text);
    }
}

static void generate_type_c(struct type *t, FILE *output) {
    if (!t) return;

    switch (t->kind) {
        case TYPE_VOID:
            fprintf(output, "void");
            break;
        case TYPE_BOOLEAN:
            fprintf(output, "int");
            break;
        case TYPE_CHARACTER:
            fprintf(output, "char");
            break;
        case TYPE_INTEGER:
            fprintf(output, "int");
            break;
        case TYPE_STRING:
            fprintf(output, "char*");
            break;
        case TYPE_ARRAY:
            generate_type_c(t->subtype, output);
            break;
        case TYPE_FUNCTION:
            generate_type_c(t->subtype, output);
            break;
    }
}

static void generate_array_initializer(struct expr *e, FILE *output) {
    if (!e) {
        return;
    }

    fprintf(output, "{");

    struct expr *element = e->right;
    if (!element) {
        fprintf(output, "0, 0, 0");
    } else {
        int first = 1;
        while (element) {
            if (!first) fprintf(output, ", ");
            generate_expr_c(element, output);
            element = element->right;
            first = 0;
        }
    }

    fprintf(output, "}");
}

static void generate_expr_c(struct expr *e, FILE *output) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INTEGER_LITERAL:
            fprintf(output, "%d", e->integer_value);
            break;
        case EXPR_STRING_LITERAL:
            fprintf(output, "\"");
            process_string_for_c(e->string_literal, output);
            fprintf(output, "\"");
            break;
        case EXPR_CHAR_LITERAL:
            fprintf(output, "'%c'", e->integer_value);
            break;
        case EXPR_BOOL_LITERAL:
            fprintf(output, "%s", e->integer_value ? "1" : "0");
            break;
        case EXPR_NAME:
            fprintf(output, "%s", e->name);
            break;
        case EXPR_CALL:
            generate_expr_c(e->left, output);
            fprintf(output, "(");

            struct expr *arg = e->right;
            while (arg) {
                generate_expr_c(arg, output);
                arg = arg->right;
                if (arg) fprintf(output, ", ");
            }

            fprintf(output, ")");
            break;
        case EXPR_SUBSCRIPT:
            generate_expr_c(e->left, output);
            fprintf(output, "[");
            generate_expr_c(e->right, output);
            fprintf(output, "]");
            break;
        case EXPR_UNARY_MINUS:
            fprintf(output, "(-");
            generate_expr_c(e->right, output);
            fprintf(output, ")");
            break;
        case EXPR_NOT:
            fprintf(output, "!");
            generate_expr_c(e->right, output);
            break;
        case EXPR_POWER:
            fprintf(output, "pow(");
            generate_expr_c(e->left, output);
            fprintf(output, ", ");
            generate_expr_c(e->right, output);
            fprintf(output, ")");
            break;
        case EXPR_ARRAY_LITERAL:
            generate_array_initializer(e, output);
            break;
        case EXPR_ASSIGN:
            generate_expr_c(e->left, output);
            fprintf(output, " = ");
            generate_expr_c(e->right, output);
            break;
        case EXPR_ARG:
            generate_expr_c(e->left, output);
            break;
        default:
            fprintf(output, "(");
            generate_expr_c(e->left, output);

            switch (e->kind) {
                case EXPR_ADD: fprintf(output, " + "); break;
                case EXPR_SUB: fprintf(output, " - "); break;
                case EXPR_MUL: fprintf(output, " * "); break;
                case EXPR_DIV: fprintf(output, " / "); break;
                case EXPR_MOD: fprintf(output, " %% "); break;
                case EXPR_EQ: fprintf(output, " == "); break;
                case EXPR_NEQ: fprintf(output, " != "); break;
                case EXPR_LT: fprintf(output, " < "); break;
                case EXPR_GT: fprintf(output, " > "); break;
                case EXPR_LE: fprintf(output, " <= "); break;
                case EXPR_GE: fprintf(output, " >= "); break;
                case EXPR_AND: fprintf(output, " && "); break;
                case EXPR_OR: fprintf(output, " || "); break;
                default: break;
            }

            generate_expr_c(e->right, output);
            fprintf(output, ")");
            break;
    }
}

static void print_indent(FILE *output, int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(output, "\t");
    }
}

static const char *get_format_specifier(struct expr *expr) {
    if (!expr) return "%d";

    if (expr->kind == EXPR_ARG && expr->left) {
        return get_format_specifier(expr->left);
    }

    switch (expr->kind) {
        case EXPR_STRING_LITERAL:
            return NULL;
        case EXPR_NAME:
            if (global_symbols) {
                type_kind_t type = lookup_symbol(global_symbols, expr->name);
                if (type == TYPE_STRING) {
                    return "%s";
                }
            }
            return "%d";
        case EXPR_SUBSCRIPT:
            if (expr->left && expr->left->kind == EXPR_NAME && global_symbols) {
                type_kind_t type = lookup_symbol(global_symbols, expr->left->name);
                if (type == TYPE_STRING) {
                    return "%c";
                }
            }
            return "%d";
        default:
            return "%d";
    }
}

static void generate_print_stmt(struct expr *expr_list, FILE *output, int indent) {
    if (!expr_list) return;

    int arg_count = 0;
    struct expr *current = expr_list;
    while (current) {
        arg_count++;
        current = current->right;
    }

    const char **format_strings = malloc(sizeof(char*) * arg_count);
    struct expr **arg_exprs = malloc(sizeof(struct expr*) * arg_count);

    current = expr_list;
    int i = 0;
    while (current && i < arg_count) {
        if (current->kind == EXPR_ARG && current->left) {
            format_strings[i] = get_format_specifier(current->left);
            arg_exprs[i] = current->left;
        } else {
            format_strings[i] = get_format_specifier(current);
            arg_exprs[i] = current;
        }
        current = current->right;
        i++;
    }

    print_indent(output, indent);
    fprintf(output, "printf(\"");

    for (i = 0; i < arg_count; i++) {
        if (format_strings[i]) {
            fprintf(output, "%s", format_strings[i]);
        } else if (arg_exprs[i] && arg_exprs[i]->kind == EXPR_STRING_LITERAL) {
            process_string_for_c(arg_exprs[i]->string_literal, output);
        }
    }
    fprintf(output, "\"");

    for (i = 0; i < arg_count; i++) {
        if (arg_exprs[i] && arg_exprs[i]->kind != EXPR_STRING_LITERAL) {
            fprintf(output, ", ");
            generate_expr_c(arg_exprs[i], output);
        }
    }

    fprintf(output, ");\n");

    free(format_strings);
    free(arg_exprs);
}

static void generate_stmt_c(struct stmt *s, FILE *output, int indent) {
    if (!s) return;

    switch (s->kind) {
        case STMT_DECL:
            print_indent(output, indent);
            generate_type_c(s->decl->type, output);
            fprintf(output, " %s", s->decl->name);

            if (s->decl->type->kind == TYPE_ARRAY) {
                fprintf(output, "[");
                generate_expr_c(s->decl->type->array_size, output);
                fprintf(output, "]");
            }

            if (s->decl->value) {
                fprintf(output, " = ");
                generate_expr_c(s->decl->value, output);
            }

            fprintf(output, ";\n");

            if (s->decl->type->kind == TYPE_ARRAY) {
                add_symbol(global_symbols, s->decl->name, s->decl->type->subtype->kind);
            } else {
                add_symbol(global_symbols, s->decl->name, s->decl->type->kind);
            }
            break;

        case STMT_EXPR:
            print_indent(output, indent);
            generate_expr_c(s->expr, output);
            fprintf(output, ";\n");
            break;

        case STMT_IF_ELSE:
            print_indent(output, indent);
            fprintf(output, "if (");
            generate_expr_c(s->expr, output);
            fprintf(output, ") {\n");

            if (s->body) {
                if (s->body->kind == STMT_BLOCK) {
                    if (s->body->body) {
                        generate_stmt_c(s->body->body, output, indent + 1);
                    }
                } else {
                    generate_stmt_c(s->body, output, indent + 1);
                }
            }

            print_indent(output, indent);
            fprintf(output, "}\n");

            if (s->else_body) {
                print_indent(output, indent);
                fprintf(output, "else {\n");

                if (s->else_body->kind == STMT_BLOCK) {
                    if (s->else_body->body) {
                        generate_stmt_c(s->else_body->body, output, indent + 1);
                    }
                } else {
                    generate_stmt_c(s->else_body, output, indent + 1);
                }

                print_indent(output, indent);
                fprintf(output, "}\n");
            }
            break;

        case STMT_FOR:
            print_indent(output, indent);
            fprintf(output, "for (");

            if (s->init_expr) {
                generate_expr_c(s->init_expr, output);
            }
            fprintf(output, "; ");

            if (s->expr) {
                generate_expr_c(s->expr, output);
            }
            fprintf(output, "; ");

            if (s->next_expr) {
                generate_expr_c(s->next_expr, output);
            }

            fprintf(output, ") {\n");

            if (s->body) {
                if (s->body->kind == STMT_BLOCK) {
                    if (s->body->body) {
                        generate_stmt_c(s->body->body, output, indent + 1);
                    }
                } else {
                    generate_stmt_c(s->body, output, indent + 1);
                }
            }

            print_indent(output, indent);
            fprintf(output, "}\n");
            break;

        case STMT_PRINT:
            generate_print_stmt(s->expr, output, indent);
            break;

        case STMT_RETURN:
            print_indent(output, indent);
            fprintf(output, "return");

            if (s->expr) {
                fprintf(output, " ");
                generate_expr_c(s->expr, output);
            }

            fprintf(output, ";\n");
            break;

        case STMT_BLOCK:
            print_indent(output, indent);
            fprintf(output, "{\n");

            if (s->body) {
                generate_stmt_c(s->body, output, indent + 1);
            }

            print_indent(output, indent);
            fprintf(output, "}\n");
            break;

        case STMT_COMMENT:
            print_indent(output, indent);
            fprintf(output, "%s\n", s->comment_text);
            break;

        case STMT_MULTI_COMMENT:
            print_indent(output, indent);
            fprintf(output, "/*\n");

            char comment_copy[4096];
            strncpy(comment_copy, s->comment_text + 2, sizeof(comment_copy) - 1);
            comment_copy[sizeof(comment_copy) - 1] = '\0';

            int len = strlen(comment_copy);
            if (len >= 2 && comment_copy[len-2] == '*' && comment_copy[len-1] == '/') {
                comment_copy[len-2] = '\0';
            }

            char *saveptr;
            char *line = strtok_r(comment_copy, "\n", &saveptr);
            while (line) {
                print_indent(output, indent);
                fprintf(output, " * %s\n", line);
                line = strtok_r(NULL, "\n", &saveptr);
            }

            print_indent(output, indent);
            fprintf(output, " */\n");
            break;
    }

    if (s->next) {
        generate_stmt_c(s->next, output, indent);
    }
}

static void generate_decl_c(struct decl *d, FILE *output) {
    if (!d) return;

    switch (d->kind) {
        case DECL_FUNCTION:
        case DECL_VARIABLE:
            if (d->type && d->type->kind == TYPE_FUNCTION) {
                generate_type_c(d->type->subtype, output);
                fprintf(output, " %s(", d->name);

                struct param_list *p = d->type->params;
                while (p) {
                    generate_type_c(p->type, output);
                    fprintf(output, " %s", p->name);

                    add_symbol(global_symbols, p->name, p->type->kind);

                    p = p->next;
                    if (p) fprintf(output, ", ");
                }

                fprintf(output, ") {\n");

                if (d->code) {
                    if (d->code->kind == STMT_BLOCK) {
                        if (d->code->body) {
                            generate_stmt_c(d->code->body, output, 1);
                        }
                    } else {
                        generate_stmt_c(d->code, output, 1);
                    }
                }

                fprintf(output, "}\n\n");
            } else {
                generate_type_c(d->type, output);

                if (d->type->kind == TYPE_ARRAY) {
                    fprintf(output, " %s[", d->name);
                    generate_expr_c(d->type->array_size, output);
                    fprintf(output, "]");

                    add_symbol(global_symbols, d->name, d->type->subtype->kind);
                } else {
                    fprintf(output, " %s", d->name);

                    add_symbol(global_symbols, d->name, d->type->kind);
                }

                if (d->value) {
                    fprintf(output, " = ");
                    generate_expr_c(d->value, output);
                }

                fprintf(output, ";\n\n");
            }
            break;

        case DECL_COMMENT:
            generate_comment(d->comment_text, output, 0);
            fprintf(output, "\n");
            break;

        case DECL_MULTI_COMMENT:
            generate_comment(d->comment_text, output, 1);
            fprintf(output, "\n");
            break;
    }

    if (d->next) {
        generate_decl_c(d->next, output);
    }
}

void generate_c_code(struct decl *program, FILE *output) {
    global_symbols = create_symbol_table();

    fprintf(output, "#include <stdio.h>\n");
    fprintf(output, "#include <stdlib.h>\n");
    fprintf(output, "#include <string.h>\n");
    fprintf(output, "#include <math.h>\n\n");

    generate_decl_c(program, output);

    free_symbol_table(global_symbols);
    global_symbols = NULL;
}