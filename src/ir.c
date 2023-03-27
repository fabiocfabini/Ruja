#include <stdio.h>
#include <stdlib.h>

#include "../includes/ir.h"


Ruja_Ir *ir_new() {
    Ruja_Ast ast = ast_new_stmt(NULL, NULL);
    if (ast == NULL) return NULL;

    Ruja_Symbol_Table *symbol_table = symbol_table_new(8);
    if (symbol_table == NULL) {
        ast_free(ast);
        return NULL;
    }

    Ruja_Ir *ir = malloc(sizeof(Ruja_Ir));
    if (ir == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for IR.\n");
        ast_free(ast);
        symbol_table_free(symbol_table);
        return NULL;
    }

    ir->ast = ast;
    ir->symbol_table = symbol_table;

    return ir;
}

void ir_free(Ruja_Ir *ir) {
    if (ir == NULL) return;

    ast_free(ir->ast);
    symbol_table_free(ir->symbol_table);
    free(ir);
}

