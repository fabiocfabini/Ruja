#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "parser.h"

/**
 * @brief Signals a lexer error to the parser.
 * 
 * @param parser The parser being signaled.
 */
static void signal_lexer_error(Ruja_Parser* parser) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;

    parser->had_error = true;
}

/**
 * @brief Signals a parser error and prints the error message.
 * 
 * @param parser The parser being signaled.
 * @param lexer The lexer that holds the source file.
 * @param msg The error message.
 */
static void parser_error(Ruja_Parser* parser, Ruja_Lexer* lexer, const char* msg) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;

    fprintf(stderr, "%s:%"PRIu64": "RED"parse error"RESET" %s got '%.*s'.\n", lexer->source, parser->current.line, msg, (int) parser->current.length, parser->current.start);
    parser->had_error = true;
}

/**
 * @brief Advances the parser to the next token.
 * 
 * @param parser The parser being advanced.
 * @param lexer The lexer that holds the source file.
 */
static void advance(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    parser->previous = parser->current;

    while (true) {
        parser->current = next_token(lexer);

        if (parser->current.kind != RUJA_TOK_ERR) break;

        signal_lexer_error(parser);
    }
}

/**
 * @brief Checks if the current token is of the given kind. If not, it signals a parser error.
 * 
 * @param parser The parser being checked.
 * @param lexer The lexer that holds the source file.
 * @param kind The kind of token to check for.
 * @param msg The error message.
 */
static void expect(Ruja_Parser* parser, Ruja_Lexer* lexer, Ruja_Token_Kind kind, const char* msg) {
    if (parser->current.kind == kind) {
        advance(parser, lexer);
        return;
    }

    parser_error(parser, lexer, msg);
}

void expression(Ruja_Parser* parser, Ruja_Lexer* lexer);

void primary(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    if (parser->current.kind == RUJA_TOK_TRUE || parser->current.kind == RUJA_TOK_FALSE || parser->current.kind == RUJA_TOK_INT || parser->current.kind == RUJA_TOK_STRING || parser->current.kind == RUJA_TOK_ID) {
        token_to_string(&parser->current);
        advance(parser, lexer);
    } else if (parser->current.kind == RUJA_TOK_LPAREN) {
        advance(parser, lexer);
        expression(parser, lexer);
        expect(parser, lexer, RUJA_TOK_RPAREN, "Expected ')' after expression.");
    } else {
        parser_error(parser, lexer, "Expected expression.");
    }
}

void unary(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    if (parser->current.kind == RUJA_TOK_NOT || parser->current.kind == RUJA_TOK_SUB) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        unary(parser, lexer);
        token_to_string(&operator);
    } else {
        primary(parser, lexer);
    }
}

void factor(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    unary(parser, lexer);

    while (parser->current.kind == RUJA_TOK_MUL || parser->current.kind == RUJA_TOK_DIV) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        unary(parser, lexer);
        token_to_string(&operator);
    }
}

void term(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    factor(parser, lexer);

    while (parser->current.kind == RUJA_TOK_ADD || parser->current.kind == RUJA_TOK_SUB) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        factor(parser, lexer);
        token_to_string(&operator);
    }
}

void comparison(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    term(parser, lexer);

    while (parser->current.kind == RUJA_TOK_GT || parser->current.kind == RUJA_TOK_GE || parser->current.kind == RUJA_TOK_LT || parser->current.kind == RUJA_TOK_LE) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        term(parser, lexer);
        token_to_string(&operator);
    }
}

void equality(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    comparison(parser, lexer);

    while (parser->current.kind == RUJA_TOK_EQ || parser->current.kind == RUJA_TOK_NE) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        comparison(parser, lexer);
        token_to_string(&operator);
    }
}

void logical_and(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    equality(parser, lexer);

    while (parser->current.kind == RUJA_TOK_AND) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        equality(parser, lexer);
        token_to_string(&operator);
    }
}

void logical_or(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    logical_and(parser, lexer);

    while (parser->current.kind == RUJA_TOK_OR) {
        Ruja_Token operator = parser->current;
        advance(parser, lexer);
        logical_and(parser, lexer);
        token_to_string(&operator);
    }
}

void expression(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    logical_or(parser, lexer);
}

bool parse(Ruja_Parser* parser, Ruja_Lexer* lexer) {
    // quick start the parser
    advance(parser, lexer);

    expression(parser, lexer);
    expect(parser, lexer, RUJA_TOK_EOF, "Expected end of file");

    return !parser->had_error;
}

Ruja_Parser* parser_new() {
    Ruja_Parser* parser = malloc(sizeof(Ruja_Parser));
    if(parser == NULL) return NULL;

    parser->had_error = false;
    parser->panic_mode = false;

    return parser;
}

void parser_free(Ruja_Parser* parser) {
    free(parser);
}

/*
expression  : logical_or
            ;

logical_or  : logical_and ( "or" logical_and )*
            ;

logical_and : equality ( "and" equality )*
            ;

equality    : comparison ( ( "!=" | "==" ) comparison )*
            ;

comparison  : term ( ( ">" | ">=" | "<" | "<=" ) term )*
            ;

term    : factor ( ( "-" | "+" ) factor )*
        ;

factor  : unary ( ( "/" | "*" ) unary )*
        ;

unary   : ( "!" | "-" ) unary
        | primary
        ;

primary : NUMBER | STRING | "true" | "false" | "(" expression ")"
        ;
*/
