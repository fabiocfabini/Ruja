#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "parser.h"

/**
 * @brief Signals a lexer error to the parser.
 *
 * @param parser The parser being signaled.
 */
static void signal_lexer_error(Ruja_Parser *parser)
{
    if (parser->panic_mode)
        return;
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
static void parser_error(Ruja_Parser *parser, Ruja_Lexer *lexer, const char *msg)
{
    if (parser->panic_mode)
        return;
    parser->panic_mode = true;

    fprintf(stderr, "%s:%" PRIu64 ": " RED "parse error" RESET " %s got '%.*s'.\n", lexer->source, parser->current.line, msg, (int)parser->current.length, parser->current.start);
    parser->had_error = true;
}

/**
 * @brief Advances the parser to the next token.
 *
 * @param parser The parser being advanced.
 * @param lexer The lexer that holds the source file.
 */
static void advance(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    parser->previous = parser->current;

    while (true)
    {
        parser->current = next_token(lexer);

        if (parser->current.kind != RUJA_TOK_ERR)
            break;

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
static void expect(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Token_Kind kind, const char *msg)
{
    if (parser->current.kind == kind)
    {
        advance(parser, lexer);
        return;
    }

    parser_error(parser, lexer, msg);
}

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_QUESTION,   // ? :
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*Parser_Function)(Ruja_Parser *, Ruja_Lexer *);
typedef struct
{
    Parser_Function prefix;
    Parser_Function infix;
    Precedence precedence;
} Parse_Rule;

static void expression(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void ternary(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void binary(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void unary(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void number_i(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void number_f(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void number_c(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void string(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void identifier(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void grouping(Ruja_Parser *parser, Ruja_Lexer *lexer);
static void parse_precedence(Ruja_Parser *parser, Ruja_Lexer *lexer, Precedence precedence);

static Parse_Rule rules[] = {
    [RUJA_TOK_LBRACE] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RBRACE] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LPAREN] = {grouping, NULL, PREC_NONE},
    [RUJA_TOK_RPAREN] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LBRACKET] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RBRACKET] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_COLON] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_COMMA] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_DOT] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_QUESTION] = {NULL, ternary, PREC_QUESTION},
    [RUJA_TOK_ASSIGN] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LT] = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_GT] = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_ADD] = {NULL, binary, PREC_TERM},
    [RUJA_TOK_SUB] = {unary, binary, PREC_TERM},
    [RUJA_TOK_MUL] = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_DIV] = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_PERCENT] = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_BANG] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_EQ] = {NULL, binary, PREC_EQUALITY},
    [RUJA_TOK_NE] = {NULL, binary, PREC_EQUALITY},
    [RUJA_TOK_LE] = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_GE] = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_ARROW] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ADD_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_SUB_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_MUL_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_DIV_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_PERCENT_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_AND] = {NULL, binary, PREC_AND},
    [RUJA_TOK_OR] = {NULL, binary, PREC_OR},
    [RUJA_TOK_NOT] = {unary, NULL, PREC_UNARY},
    [RUJA_TOK_IF] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ELSE] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ELIF] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_FOR] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_IN] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_PROC] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RETURN] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_STRUCT] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ENUM] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TRUE] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_FALSE] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LET] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ID] = {identifier, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_I8] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_INT] = {number_i, NULL, PREC_NONE},
    [RUJA_TOK_FLOAT] = {number_f, NULL, PREC_NONE},
    [RUJA_TOK_STRING] = {string, NULL, PREC_NONE},
    [RUJA_TOK_CHAR] = {number_c, NULL, PREC_NONE},
};

static Parse_Rule *get_rule(Ruja_Token_Kind kind)
{
    return &rules[kind];
}

static void number_i(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // Convert the token to the correct type an emit the corresponding bytecode
    // NOTE: For now the bytecode is not implemented so just print the token
    token_to_string(&parser->previous);
}

static void number_f(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // Convert the token to the correct type an emit the corresponding bytecode
    // NOTE: For now the bytecode is not implemented so just print the token
    token_to_string(&parser->previous);
}

static void number_c(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // Convert the token to the correct type an emit the corresponding bytecode
    // NOTE: For now the bytecode is not implemented so just print the token
    token_to_string(&parser->previous);
}

static void string(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // Convert the token to the correct type an emit the corresponding bytecode
    // NOTE: For now the bytecode is not implemented so just print the token
    token_to_string(&parser->previous);
}

static void identifier(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // Convert the token to the correct type an emit the corresponding bytecode
    // NOTE: For now the bytecode is not implemented so just print the token
    token_to_string(&parser->previous);
}

static void unary(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    assert((parser->previous.kind == RUJA_TOK_NOT || parser->previous.kind == RUJA_TOK_SUB) &&
            "This function assumes that a unary token has been already consumed.");

    // Save the previous unary operation
    Ruja_Token unary_op = parser->previous;

    // Parse any following expressions that have equal or higher precedence
    parse_precedence(parser, lexer, PREC_UNARY);

    // Emit byte code fot the correct operation
    // NOTE: For now bytecode is not implemented just print the token
    token_to_string(&unary_op);
}

static void binary(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    assert((parser->previous.kind == RUJA_TOK_SUB || parser->previous.kind == RUJA_TOK_ADD ||
            parser->previous.kind == RUJA_TOK_DIV || parser->previous.kind == RUJA_TOK_MUL ||
            parser->previous.kind == RUJA_TOK_AND || parser->previous.kind == RUJA_TOK_OR  ||
            parser->previous.kind == RUJA_TOK_EQ  || parser->previous.kind == RUJA_TOK_NE  ||
            parser->previous.kind == RUJA_TOK_GT  || parser->previous.kind == RUJA_TOK_LT  ||
            parser->previous.kind == RUJA_TOK_GE  || parser->previous.kind == RUJA_TOK_LE) &&
            "This function assumes that a binary token has been already consumed.");

    // Save the current binary operation
    Ruja_Token binary_op = parser->previous;

    // Parse any following expressions that have higher precedence
    // Since not all binary operations have the same precedence we must search for it
    Precedence binary_op_precedence = get_rule(parser->previous.kind)->precedence;
    parse_precedence(parser, lexer, binary_op_precedence + 1);

    // Emit byte code fot the correct operation
    // NOTE: For now bytecode is not implemented just print the token
    token_to_string(&binary_op);
}

static void ternary(Ruja_Parser *parser, Ruja_Lexer *lexer) {
    assert( parser->previous.kind == RUJA_TOK_QUESTION &&
            "This function assumes that a ternary token has been already consumed.");

    token_to_string(&parser->previous);
    expression(parser, lexer);

    assert( parser->current.kind == RUJA_TOK_COLON &&
            "This function assumes that a ternary token has been already consumed.");

    advance(parser, lexer);
    token_to_string(&parser->previous);
    expression(parser, lexer);
}

static void grouping(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    expression(parser, lexer);
    expect(parser, lexer, RUJA_TOK_RPAREN, "Unclosed left parenthesis. Expected ')'");
}

static void expression(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    parse_precedence(parser, lexer, PREC_ASSIGNMENT);
}

static void parse_precedence(Ruja_Parser *parser, Ruja_Lexer *lexer, Precedence precedence)
{
    // At this point in the execution the current token can only be a token that has prefix rules (unary, primary or '(')
    // Get the current token's prefix rule
    advance(parser, lexer);
    Parser_Function prefix_rule = get_rule(parser->previous.kind)->prefix;
    if (prefix_rule == NULL)
    {
        parser_error(parser, lexer, "Expected an expression");
        return;
    }

    // Consume the current token since prefix rules expect the token to be in previous
    // Execute the obtained prefix rule
    prefix_rule(parser, lexer);

    /*
    At this point there are 3 possible outcomes that have the same resolution those are.

    1 - The prefix function we performed was the grouping function.
        Remember that function calls 'expression' so when we get back to this function
        the entire expression inside parenthesis has been parsed. This means that the
        the current token may not even be an expression (It can be an EOF or keyword).
        If this is the case then our expression is finished and therefore we don't want
        to do anything else. If the current token is part of an expression than it will
        have an associated precedence. We only want to parse it if its precedence is less
        or equal to 'precedence' parameter

    2 - The prefix function we performed was the unary function.
        This means that the token that was read was (so far) either 
        'not' or '-'. Remember that the function also calls parse_precedence
        and this time every operation that has equal or higher precedence
        has already been parsed. Once again, the current token (once we come 
        back to this function) will be an expression or not. This means that 
        we also just want to parse the current token if its precedence is equal
        or higher than the 'precedence' parameter.

    3 - The prefix function we performed was the one of the primary prefix functions
        (number_i, number_f, number_c, string, identifier). These functions only parse
        one token which means that (once again) we are in the situation that the current
        token is either an expression in which case it has an associated precedence or it
        is not an expression and has no precedence whatsoever. We do exactly the same has
        before and only parse the current token if it has less precedence than the 'precedence'
        parameter.
    */

    while (precedence <= get_rule(parser->current.kind)->precedence) {
        advance(parser, lexer);
        get_rule(parser->previous.kind)->infix(parser, lexer);
    }
}

bool parse(Ruja_Parser *parser, Ruja_Lexer *lexer)
{
    // quick start the parser
    advance(parser, lexer);

    expression(parser, lexer);
    expect(parser, lexer, RUJA_TOK_EOF, "Expected end of file");

    return !parser->had_error;
}

Ruja_Parser *parser_new()
{
    Ruja_Parser *parser = malloc(sizeof(Ruja_Parser));
    if (parser == NULL)
        return NULL;

    parser->had_error = false;
    parser->panic_mode = false;

    return parser;
}

void parser_free(Ruja_Parser *parser)
{
    free(parser);
}
