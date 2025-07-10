#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char *argv[]) {
    char input_file_name[256];
    if (argc < 2) {
        strcpy(input_file_name, "example.b");
    } else {
        strcpy(input_file_name, argv[1]);
    }
    
    FILE *input_file = fopen(input_file_name, "r");
    if (!input_file) {
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char *input = malloc(file_size + 1);
    fread(input, 1, file_size, input_file);
    input[file_size] = '\0';
    fclose(input_file);

    lexer_t *lexer = init_lexer(input);
    parser_t *parser = init_parser(lexer);
    struct decl *program = parse_program(parser);

    if (program) {
        char output_filename[256];
        snprintf(output_filename, sizeof(output_filename), "%s.c", input_file_name);

        FILE *output_file = fopen(output_filename, "w");
        if (output_file) {
            generate_c_code(program, output_file);
            fclose(output_file);
        }
    }

    free_parser(parser);
    free(input);

    return 0;
}