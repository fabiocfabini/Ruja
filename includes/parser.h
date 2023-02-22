#ifndef RUJA_PARSER_H
#define RUJA_PARSER_H

#include "common.h"
#include "lexer.h"


typedef enum {
    RUJA_PARSER_ERROR,
    RUJA_PARSER_OK,
} Ruja_Parse_Error;

typedef struct {
    Ruja_Token previous;
    Ruja_Token current;
    bool had_error;
    bool panic_mode;
} Ruja_Parser;

Ruja_Parser* parser_new();
void parser_free(Ruja_Parser* parser);
bool parse(Ruja_Parser* parser, Ruja_Lexer* lexer);

#endif // RUJA_PARSER_H