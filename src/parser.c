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
static void parser_error(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Token* token, const char *msg) {
    if (parser->panic_mode)
        return;
    parser->panic_mode = true;

    fprintf(stderr, "%s:%" PRIu64 ": " RED "parse error" RESET " %s got '%.*s'.\n", lexer->source, token->line, msg, (int)token->length, token->start);
    parser->had_error = true;
}

static void maybe_free_token(Ruja_Token *token) {
    if (!token->in_ast) token_free(token);
}

/**
 * @brief Advances the parser to the next token.
 *
 * @param parser The parser being advanced.
 * @param lexer The lexer that holds the source file.
 */
static void advance(Ruja_Parser *parser, Ruja_Lexer *lexer) {
    if (parser->previous != NULL) maybe_free_token(parser->previous);

    parser->previous = parser->current;

    while (true) {
        parser->current = next_token(lexer);

        if (parser->current->kind != RUJA_TOK_ERR)
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
    if (parser->current->kind == kind) {
        advance(parser, lexer);
        return;
    }

    parser_error(parser, lexer, parser->current, msg);
}

static void expect_either(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Token_Kind expected[2], const char *msg) {
    if (parser->current->kind == expected[0] || parser->current->kind == expected[1]) {
        advance(parser, lexer);
        return;
    }

    parser_error(parser, lexer, parser->current, msg);
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
    [RUJA_TOK_IF]         = {NULL, ternary, PREC_QUESTION},
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
    [RUJA_TOK_BREAK]      = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_CONTINUE]   = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_I32]   = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_F64]   = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_BOOL]  = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_CHAR]  = {NULL, NULL, PREC_NONE},
    [RUJA_TOK_TYPE_STRING]= {NULL, NULL, PREC_NONE},
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

    (*ast) = ast_new_literal(parser->previous);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_TRUE or RUJA_TOK_FALSE
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void boolean(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    (*ast) = ast_new_literal(parser->previous);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_INT
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void integer(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    (*ast) = ast_new_literal(parser->previous);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_FLOAT 
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void floating(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    (*ast) = ast_new_literal(parser->previous);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_CHAR
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void character(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    (*ast) = ast_new_literal(parser->previous);
}

/**
 * @brief Parse a Token of kind RUJA_TOK_STRING
 * 
 * @param parser The Parser in use
 * @param lexer The Lexer is use
 */
static void string(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast* ast) {
    UNUSED(lexer);
    
    (*ast) = ast_new_literal(parser->previous);
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
    
    (*ast) = ast_new_identifier(parser->previous);
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
    Ruja_Token* unary_op = parser->previous;
    Ruja_Ast unary = ast_new_unary_op(unary_op, NULL);

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
    Ruja_Token* binary_op = parser->previous;
    Ruja_Ast binary = ast_new_binary_op(binary_op, *ast, NULL);

    // Parse any following expressions that have higher precedence
    // Since not all binary operations have the same precedence we must search for it
    Precedence binary_op_precedence = get_rule(parser->previous->kind)->precedence;
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

    Ruja_Ast ternary = ast_new_ternary_op(parser->previous, NULL, *ast, NULL, NULL);

    expression(parser, lexer, &ternary->as.ternary_op.true_expression);

    // assert( parser->current.kind == RUJA_TOK_COLON &&
    //         "This function assumes that a ternary token has been already consumed.");
    Ruja_Token_Kind expected[] = {RUJA_TOK_COLON, RUJA_TOK_ELSE};
    expect_either(parser, lexer, expected, "Expected ':' or 'else' after ternary operator '?'/'if'");
    if (!parser->had_error) {
        // If an error occurred, it means that the previous token was not a colon nor an else
        // We must not allow the previous token to be in the AST as a tok_ternary.tok_colon
        // A risk of double free would occur in case the previous token was a literal or identifier
        // Try with this input: isAlive = name == 1 if y;
        // The double free should occur in the y token
        parser->previous->in_ast = true;
        ternary->as.ternary_op.tok_ternary.tok_colon = parser->previous;

        expression(parser, lexer, &ternary->as.ternary_op.false_expression);
    }

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
    Parser_Function prefix_rule = get_rule(parser->previous->kind)->prefix;
    if (prefix_rule == NULL)
    {
        parser_error(parser, lexer, parser->previous, "Expected an expression");
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

    while (precedence <= get_rule(parser->current->kind)->precedence) {
        advance(parser, lexer);
        Parser_Function infix_rule = get_rule(parser->previous->kind)->infix;
        if (infix_rule == NULL) {
            parser_error(parser, lexer, parser->previous, "Expected binary operator");
            return;
        }
        infix_rule(parser, lexer, ast);
    }
}

static void typed_declaration(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // previous is the identifier and current is the colon
    Ruja_Token* tok_id = parser->previous;
    tok_id->in_ast = true; // signal that this token should be in the AST. If something goes wrong it is this function's responsibility to free it
    advance(parser, lexer);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wswitch-enum"
    // the next token must be a type
    switch (parser->current->kind) {
        case RUJA_TOK_TYPE_BOOL:
        case RUJA_TOK_TYPE_CHAR:
        case RUJA_TOK_TYPE_I32:
        case RUJA_TOK_TYPE_F64:
        case RUJA_TOK_TYPE_STRING: {
            advance(parser, lexer);
            // the next token must be either an equal sign or a semicolon
            switch (parser->current->kind) {
                case RUJA_TOK_SEMICOLON: {
                    // This is a typed declaration
                    *ast = ast_new_typed_decl(parser->previous, ast_new_identifier(tok_id));
                } break;
                case RUJA_TOK_ASSIGN:
                case RUJA_TOK_ADD_EQ:
                case RUJA_TOK_SUB_EQ:
                case RUJA_TOK_MUL_EQ:
                case RUJA_TOK_DIV_EQ: {
                    // This is a typed declaration with an assignment
                    *ast = ast_new_typed_decl_assign(parser->previous, parser->current, ast_new_identifier(tok_id), ast_new_expression(NULL));
                    advance(parser, lexer);
                    expression(parser, lexer, &(*ast)->as.typed_decl_assign.expression->as.expr.expression);
                } break;
                default: {
                    parser_error(parser, lexer, parser->current, "Expected '=' or ';' after type");
                    token_free(tok_id);
                    return;
                } break;
            }
        } break;
        default: {
            parser_error(parser, lexer, parser->current, "Expected type after ':'");
            token_free(tok_id);
            return;
        } break;
    }
#pragma GCC diagnostic pop
}

static void inferred_declaration(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // previous is the identifier and current is the equal sign
    Ruja_Token* tok_id = parser->previous;
    tok_id->in_ast = true; // signal that this token should be in the AST. If something goes wrong it is this function's responsibility to free it
    advance(parser, lexer);

    // the next token must be an expression
    *ast = ast_new_inferred_decl_assign(parser->previous, ast_new_identifier(tok_id), ast_new_expression(NULL));
    expression(parser, lexer, &(*ast)->as.inferred_decl_assign.expression->as.expr.expression);
}

static void declaration(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    expect(parser, lexer, RUJA_TOK_ID, "Expected identifier after 'let' keyword");
    if (!parser->had_error) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (parser->current->kind) {
            case RUJA_TOK_COLON: {
                typed_declaration(parser, lexer, ast);
            } break;
            case RUJA_TOK_ASSIGN:
            case RUJA_TOK_ADD_EQ:
            case RUJA_TOK_SUB_EQ:
            case RUJA_TOK_MUL_EQ:
            case RUJA_TOK_DIV_EQ:{
                // This is an inferred declaration with an assignment
                inferred_declaration(parser, lexer, ast);
            } break;
            default: {
                parser_error(parser, lexer, parser->current, "Must specify type of variable. Expected ':' followed by a type");
            } break;
        }
    #pragma GCC diagnostic pop
    }
}

static void assignment(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (parser->current->kind) {
        case RUJA_TOK_ASSIGN:
        case RUJA_TOK_ADD_EQ:
        case RUJA_TOK_SUB_EQ:
        case RUJA_TOK_MUL_EQ:
        case RUJA_TOK_DIV_EQ: {
            *ast = ast_new_assign(parser->current, ast_new_identifier(parser->previous), ast_new_expression(NULL));
            advance(parser, lexer);
            expression(parser, lexer, &(*ast)->as.assign.expression->as.expr.expression);
        } break;
        default: {
            parser_error(parser, lexer, parser->current, "Expected assignment operator");
        } break;
    }
#pragma GCC diagnostic pop
}

static void statement(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    advance(parser, lexer);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (parser->previous->kind) {
        case RUJA_TOK_LET: {
            declaration(parser, lexer, ast);
            expect(parser, lexer, RUJA_TOK_SEMICOLON, "Expected ';' after declaration");
        } break;
        case RUJA_TOK_ID: {
            assignment(parser, lexer, ast);
            expect(parser, lexer, RUJA_TOK_SEMICOLON, "Expected ';' after assignment");
        } break;
        default: {
            parser_error(parser, lexer, parser->previous, "Expected a statement");
        } break;
    }
#pragma GCC diagnostic pop
}

static void statements(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    Ruja_Ast *current = ast;
    while (parser->current->kind != RUJA_TOK_EOF) {
        statement(parser, lexer, &(*current)->as.stmts.statement);
        (*current)->as.stmts.next = ast_new_stmt(NULL, NULL);
        current = &(*current)->as.stmts.next;
    }

    // If the last statement was a statement with no next, then we need to free it
    if ((*current)->as.stmts.next == NULL) {
        ast_free(*current);
        *current = NULL;
    }
}

bool parse(Ruja_Parser *parser, Ruja_Lexer *lexer, Ruja_Ast *ast) {
    // quick start the parser
    advance(parser, lexer);
    if (parser->had_error) return false;

    // We know that the root of the AST will be an expression
    if (*ast == NULL) {
        fprintf(stderr, "Null AST passed to parser\n");
        return false;
    }
    statements(parser, lexer, ast);
    expect(parser, lexer, RUJA_TOK_EOF, "Expected end of file");
    maybe_free_token(parser->previous);
    maybe_free_token(parser->current);

    return !parser->had_error;
}

Ruja_Parser *parser_new() {
    Ruja_Parser *parser = malloc(sizeof(Ruja_Parser));
    if (parser == NULL) {
        fprintf(stderr, "Failed to allocate memory for parser\n");
        return NULL;
    }

    parser->previous = NULL;
    parser->current = NULL;
    parser->had_error = false;
    parser->panic_mode = false;

    return parser;
}

void parser_free(Ruja_Parser *parser) {
    free(parser);
}
