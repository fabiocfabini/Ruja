#ifndef RUJA_PARSER_H
#define RUJA_PARSER_H

#include "common.h"
#include "lexer.h"
#include "ir.h"

typedef enum {
    RUJA_PARSER_ERROR,
    RUJA_PARSER_OK,
} Ruja_Parse_Error;


typedef struct _tstack Type_Stack;
typedef struct {
    Ruja_Token* previous;
    Ruja_Token* current;

    bool had_error;
    bool panic_mode;

    Type_Stack* type_stack;
} Ruja_Parser;

Ruja_Parser* parser_new();
void parser_free(Ruja_Parser* parser);
bool parse(Ruja_Parser* parser, Ruja_Lexer* lexer, Ruja_Ast* ast, Ruja_Symbol_Table* sb);

#endif // RUJA_PARSER_H