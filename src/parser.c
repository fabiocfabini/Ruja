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
