#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

void generate_c_code(struct decl *program, FILE *output);

#endif