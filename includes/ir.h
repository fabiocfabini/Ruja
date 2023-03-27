#ifndef RUJA_IR_H
#define RUJA_IR_H

#include "common.h"
#include "ast.h"
#include "symbol_table.h"


typedef struct {
    Ruja_Ast ast;
    Ruja_Symbol_Table *symbol_table;
} Ruja_Ir;

Ruja_Ir *ir_new();
void ir_free(Ruja_Ir *ir);

#endif // RUJA_IR_H