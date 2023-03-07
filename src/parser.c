#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "../includes/parser.h"

/**
 * @brief Signals a lexer error to the parser.
 *
 * @param parser The parser being signaled.
 */
static void signal_lexer_error(Ruja_Parser *parser) {
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
static void parser_error(Ruja_Parser *parser, Ruja_Lexer *lexer, const char *msg) {
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
static void advance(Ruja_Parser *parser, Ruja_Lexer *lexer) {
    parser->previous = parser->current;

    while (true) {
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
static void expect(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Token_Kind kind, const char *msg) {
    if (parser->current.kind == kind) {
        advance(parser, lexer);
        return;
    }

    parser_error(parser, lexer, msg);
}

typedef enum {
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

typedef void (*Parser_Function)(Ruja_Parser *, Ruja_Lexer *, Ruja_Ast *);
typedef struct {
    Parser_Function prefix;
    Parser_Function infix;
    Precedence precedence;
} Parse_Rule;

static void expression(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void ternary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void binary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void unary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void integer(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void floating(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void character(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void boolean(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void nil(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void string(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void identifier(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void grouping(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast);
static void parse_precedence(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast, Precedence precedence);

static Parse_Rule rules[] = {
    [RUJA_TOK_LBRACE]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RBRACE]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LPAREN]     = {grouping, NULL, PREC_NONE},
    [RUJA_TOK_RPAREN]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LBRACKET]   = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RBRACKET]   = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_COLON]      = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_SEMICOLON]  = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_COMMA]      = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_DOT]        = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_QUESTION]   = {NULL, ternary, PREC_QUESTION},
    [RUJA_TOK_ASSIGN]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_LT]         = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_GT]         = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_ADD]        = {NULL, binary, PREC_TERM},
    [RUJA_TOK_SUB]        = {unary, binary, PREC_TERM},
    [RUJA_TOK_MUL]        = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_DIV]        = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_PERCENT]    = {NULL, binary, PREC_FACTOR},
    [RUJA_TOK_BANG]       = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_EQ]         = {NULL, binary, PREC_EQUALITY},
    [RUJA_TOK_NE]         = {NULL, binary, PREC_EQUALITY},
    [RUJA_TOK_LE]         = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_GE]         = {NULL, binary, PREC_COMPARISON},
    [RUJA_TOK_ARROW]      = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ADD_EQ]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_SUB_EQ]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_MUL_EQ]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_DIV_EQ]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_PERCENT_EQ] = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_AND]        = {NULL, binary, PREC_AND},
    [RUJA_TOK_OR]         = {NULL, binary, PREC_OR},
    [RUJA_TOK_NOT]        = {unary, NULL, PREC_UNARY},
    [RUJA_TOK_IF]         = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ELSE]       = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ELIF]       = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_FOR]        = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_IN]         = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_PROC]       = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_RETURN]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_STRUCT]     = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ENUM]       = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_NIL]        = {nil, NULL, PREC_NONE},
    [RUJA_TOK_TRUE]       = {boolean, NULL, PREC_NONE},
    [RUJA_TOK_FALSE]      = {boolean, NULL, PREC_NONE},
    [RUJA_TOK_LET]        = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_ID]         = {identifier, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_I8]    = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_INT]        = {integer, NULL, PREC_NONE},
    [RUJA_TOK_FLOAT]      = {floating, NULL, PREC_NONE},
    [RUJA_TOK_STRING]     = {string, NULL, PREC_NONE},
    [RUJA_TOK_CHAR]       = {character, NULL, PREC_NONE},
};

/**
 * @brief Get the parsing rule associate with a given token kind
 * 
 * @param kind The kind of the token
 * @return Parse_Rule* A pointer to the correct Parse_Rule object
 */
static Parse_Rule *get_rule(Ruja_Token_Kind kind) {
    return kind >= 0 ? &rules[kind]: &rules[0]; // Rule 0 has all NULL pointers it can act as a default
}

/**
 * @brief Parse a Token of kind RUJA_TOK_NIL
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void nil(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(parser);
    UNUSED(lexer);

    Word nil = MAKE_NIL();
    (*ast) = ast_new_literal(nil);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_TRUE or RUJA_TOK_FALSE
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void boolean(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    Word boolean = MAKE_BOOL(parser->previous.start[0] == 't');
    (*ast) = ast_new_literal(boolean);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_INT
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void integer(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    Word integer = MAKE_INT(strtod(parser->previous.start, NULL));
    (*ast) = ast_new_literal(integer);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_FLOAT 
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void floating(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    Word floating = MAKE_DOUBLE(strtod(parser->previous.start, NULL));
    (*ast) = ast_new_literal(floating);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_CHAR
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void character(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    Word character = MAKE_CHAR(parser->previous.start[0]);
    (*ast) = ast_new_literal(character);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_STRING
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void string(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    UNUSED(parser);
    UNUSED(ast);
    
    NOT_IMPLEMENTED("String parsing", __FILE__, __LINE__);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_ID
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void identifier(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    UNUSED(parser);
    UNUSED(ast);
    
    NOT_IMPLEMENTED("Identifier parsing", __FILE__, __LINE__);
}

/**
 * @brief Parser an expression involved in paren
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 */
static void grouping(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    expression(parser, lexer, ast);
    expect(parser, lexer, RUJA_TOK_RPAREN, "Unclosed left parenthesis. Expected ')'");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

/**
 * @brief Convert a token unary operator to an ast unary operator
 * 
 * @param kind The token kind
 * @return ast_unary_op_type The ast unary operator 
 */
static ast_unary_op_type token_unary_to_ast_unary(Ruja_Token_Kind kind) {
    switch (kind) {
        case RUJA_TOK_NOT: return AST_UNARY_OP_NOT;
        case RUJA_TOK_SUB: return AST_UNARY_OP_NEG;
        default: assert(false && "Invalid binary operator");
    }
}

/**
 * @brief Convert a token binary operator to an ast binary operator
 * 
 * @param kind The token kind
 * @return ast_binary_op_type The ast binary operator 
 */
static ast_binary_op_type token_binary_to_ast_binary(Ruja_Token_Kind kind) {
    switch (kind) {
        case RUJA_TOK_ADD: return AST_BINARY_OP_ADD;
        case RUJA_TOK_SUB: return AST_BINARY_OP_SUB;
        case RUJA_TOK_MUL: return AST_BINARY_OP_MUL;
        case RUJA_TOK_DIV: return AST_BINARY_OP_DIV;
        case RUJA_TOK_AND: return AST_BINARY_OP_AND;
        case RUJA_TOK_OR:  return AST_BINARY_OP_OR;
        case RUJA_TOK_EQ:  return AST_BINARY_OP_EQ;
        case RUJA_TOK_NE:  return AST_BINARY_OP_NE;
        case RUJA_TOK_GT:  return AST_BINARY_OP_GT;
        case RUJA_TOK_LT:  return AST_BINARY_OP_LT;
        case RUJA_TOK_GE:  return AST_BINARY_OP_GE;
        case RUJA_TOK_LE:  return AST_BINARY_OP_LE;
        default: assert(false && "Invalid binary operator");
    }
}
#pragma GCC diagnostic pop

/**
 * @brief Parser a unary expression
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 */
static void unary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    // assert((parser->previous.kind == RUJA_TOK_NOT || parser->previous.kind == RUJA_TOK_SUB) &&
    //         "This function assumes that a unary token has been already consumed.");

    // Save the previous unary operation
    Ruja_Token unary_op = parser->previous;
    Ruja_Ast unary = ast_new_unary_op(token_unary_to_ast_unary(unary_op.kind), NULL);

    // Parse any following expressions that have equal or higher precedence
    parse_precedence(parser, lexer, &unary->as.unary_op.expression, PREC_UNARY);

    *ast = unary;
}

/**
 * @brief Parses a binary expression
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 */
static void binary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    // assert((parser->previous.kind == RUJA_TOK_SUB || parser->previous.kind == RUJA_TOK_ADD ||
    //         parser->previous.kind == RUJA_TOK_DIV || parser->previous.kind == RUJA_TOK_MUL ||
    //         parser->previous.kind == RUJA_TOK_AND || parser->previous.kind == RUJA_TOK_OR  ||
    //         parser->previous.kind == RUJA_TOK_EQ  || parser->previous.kind == RUJA_TOK_NE  ||
    //         parser->previous.kind == RUJA_TOK_GT  || parser->previous.kind == RUJA_TOK_LT  ||
    //         parser->previous.kind == RUJA_TOK_GE  || parser->previous.kind == RUJA_TOK_LE) &&
    //         "This function assumes that a binary token has been already consumed.");

    // Save the current binary operation
    Ruja_Token binary_op = parser->previous;
    Ruja_Ast binary = ast_new_binary_op(token_binary_to_ast_binary(binary_op.kind), *ast, NULL);

    // Parse any following expressions that have higher precedence
    // Since not all binary operations have the same precedence we must search for it
    Precedence binary_op_precedence = get_rule(parser->previous.kind)->precedence;
    parse_precedence(parser, lexer, &binary->as.binary_op.right_expression, binary_op_precedence + 1);

    (*ast) = binary;
}

/**
 * @brief Parses a ternary expression
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 */
static void ternary(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // At this point the ast is the expression branch of the AST_NODE_EXPRESSION node
    // this needs to be changed to the condition branch of the AST_NODE_TERNARY node
    // assert( parser->previous.kind == RUJA_TOK_QUESTION &&
    //         "This function assumes that a ternary token has been already consumed.");

    Ruja_Ast ternary = ast_new_ternary_op(*ast, NULL, NULL);

    expression(parser, lexer, &ternary->as.ternary_op.true_expression);

    // assert( parser->current.kind == RUJA_TOK_COLON &&
    //         "This function assumes that a ternary token has been already consumed.");
    expect(parser, lexer, RUJA_TOK_COLON, "Expected ':' after ternary operator '?'");

    expression(parser, lexer, &ternary->as.ternary_op.false_expression);

    (*ast) = ternary;
}

/**
 * @brief Top function of expression parsing. All parsing of expression should originate
 *      in this function
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 */
static void expression(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // At this point ast is the expression branch of the AST_NODE_EXPRESSION node
    parse_precedence(parser, lexer, ast, PREC_ASSIGNMENT);
}

/**
 * @brief Given a Precedence level, it begins by calling the prefix parsing function 
 *      of the 'parser->previous' token and follows with all the infix parsing functions
 *      of the following Ruja_Token's that have less or equal precedence than the one given.
 * 
 * @param parser The parser in use
 * @param lexer The lexer in use
 * @param precedence The level of precedence given by the function caller
 */
static void parse_precedence(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast, Precedence precedence) {
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
    prefix_rule(parser, lexer, ast);

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
        (integer, floating, character, string, identifier). These functions only parse
        one token which means that (once again) we are in the situation that the current
        token is either an expression in which case it has an associated precedence or it
        is not an expression and has no precedence whatsoever. We do exactly the same has
        before and only parse the current token if it has less precedence than the 'precedence'
        parameter.
    */

    while (precedence <= get_rule(parser->current.kind)->precedence) {
        advance(parser, lexer);
        get_rule(parser->previous.kind)->infix(parser, lexer, ast);
    }
}

bool parse(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // quick start the parser
    advance(parser, lexer);
    if (parser->had_error) return false;

    // We know that the root of the AST will be an expression
    (*ast)->type = AST_NODE_EXPRESSION;
    expression(parser, lexer, &(*ast)->as.expr.expression);
    expect(parser, lexer, RUJA_TOK_EOF, "Expected end of file");

    return !parser->had_error;
}

Ruja_Parser *parser_new() {
    Ruja_Parser *parser = malloc(sizeof(Ruja_Parser));
    if (parser == NULL) {
        fprintf(stderr, "Failed to allocate memory for parser\n");
        return NULL;
    }

    parser->had_error = false;
    parser->panic_mode = false;

    return parser;
}

void parser_free(Ruja_Parser *parser) {
    free(parser);
}
