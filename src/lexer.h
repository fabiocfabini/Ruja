#ifndef RUJA_LEXER_H
#define RUJA_LEXER_H

#include "common.h"

typedef enum {
    RUJA_TOK_EOF = -1,

    // One character
    // {}()[]:;,.=<>+-*/%
    RUJA_TOK_LBRACE, RUJA_TOK_RBRACE, RUJA_TOK_LPAREN, RUJA_TOK_RPAREN, RUJA_TOK_LBRACKET, RUJA_TOK_RBRACKET, 
    RUJA_TOK_COLON, RUJA_TOK_SEMICOLON, RUJA_TOK_COMMA, RUJA_TOK_DOT, RUJA_TOK_EQUAL, RUJA_TOK_LT, RUJA_TOK_GT,
    RUJA_TOK_ADD, RUJA_TOK_SUB, RUJA_TOK_MUL, RUJA_TOK_DIV, RUJA_TOK_PERCENT,

    // Two characters
    // == <= >= -> += -= *= /= %=
    RUJA_TOK_EQ, RUJA_TOK_LE, RUJA_TOK_GE, 
    RUJA_TOK_ADD_EQ, RUJA_TOK_SUB_EQ, RUJA_TOK_MUL_EQ, RUJA_TOK_DIV_EQ, RUJA_TOK_PERCENT_EQ, 

    // keywords
    // and or not -- if else elif -- for in -- proc return struct enum -- true false
    RUJA_TOK_AND, RUJA_TOK_OR, RUJA_TOK_NOT,
    RUJA_TOK_IF, RUJA_TOK_ELSE, RUJA_TOK_ELIF,
    RUJA_TOK_FOR, RUJA_TOK_IN,
    RUJA_TOK_PROC, RUJA_TOK_RETURN, 
    RUJA_TOK_STRUCT, RUJA_TOK_ENUM,
    RUJA_TOK_TRUE, RUJA_TOK_FALSE,
    RUJA_TOK_LET,
    RUJA_TOK_ID,

    // literals
    // 123 123.456 "hello" 'c'
    RUJA_TOK_INT, RUJA_TOK_FLOAT, RUJA_TOK_STRING, RUJA_TOK_CHAR,
} Ruja_Token_Kind;

typedef struct {
    Ruja_Token_Kind kind;
    const char *start;
    size_t length;
    size_t line;
} Ruja_Token;

void token_to_string(Ruja_Token* token);

typedef struct {
    const char *start;
    const char *current;
    size_t line;
} Ruja_Lexer;

Ruja_Lexer lexer_new(const char *source);
Ruja_Lexer lexer_free(Ruja_Lexer *lexer);
Ruja_Token next_token(Ruja_Lexer *lexer);

#endif // RUJA_LEXER_H