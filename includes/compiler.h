#ifndef RUJA_COMPILER_H
#define RUJA_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "ast.h"


typedef enum {
    RUJA_COMPILER_OK,
    RUJA_COMPILER_ERROR,
} Ruja_Compile_Error;

typedef struct {
    Ruja_Ast ast;
} Ruja_Compiler;

Ruja_Compiler* compiler_new();
void compiler_free(Ruja_Compiler *compiler);

Ruja_Compile_Error compile(Ruja_Compiler *compiler, const char *source_path, Bytecode *bytecode);

#endif // RUJA_COMPILER_H