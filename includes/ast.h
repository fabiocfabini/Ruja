#ifndef RUJA_AST_H
#define RUJA_AST_H

#include <stdio.h>

#include "common.h"
#include "lexer.h"
#include "word.h"

typedef enum {
    AST_UNARY_OP_NEG,
    AST_UNARY_OP_NOT,
} ast_unary_op_type;

const char *ast_unary_op_type_to_string(ast_unary_op_type type);

typedef enum {
    AST_BINARY_OP_ADD,
    AST_BINARY_OP_SUB,
    AST_BINARY_OP_MUL,
    AST_BINARY_OP_DIV,
    AST_BINARY_OP_EQ,
    AST_BINARY_OP_NE,
    AST_BINARY_OP_LT,
    AST_BINARY_OP_LE,
    AST_BINARY_OP_GT,
    AST_BINARY_OP_GE,
    AST_BINARY_OP_AND,
    AST_BINARY_OP_OR,
} ast_binary_op_type;

const char *ast_binary_op_type_to_string(ast_binary_op_type type);

typedef enum {
    AST_NODE_EMPTY,
    AST_NODE_LITERAL,
    AST_NODE_UNARY_OP,
    AST_NODE_BINARY_OP,
    AST_NODE_TERNARY_OP,
    AST_NODE_EXPRESSION,
} ast_node_type;

typedef struct Ruja_Ast_Node {
    ast_node_type type;
    union {
        struct {
            Ruja_Token* tok_literal;
        } literal;
        struct {
            Ruja_Token* tok_unary;
            struct Ruja_Ast_Node *expression;
        } unary_op;
        struct {
            Ruja_Token* tok_binary;
            struct Ruja_Ast_Node *left_expression;
            struct Ruja_Ast_Node *right_expression;
        } binary_op;
        struct {
            struct {
                Ruja_Token* tok_question;
                Ruja_Token* tok_colon;
            } tok_ternary;
            struct Ruja_Ast_Node *condition;
            struct Ruja_Ast_Node *true_expression;
            struct Ruja_Ast_Node *false_expression;
        } ternary_op;
        struct {
            struct Ruja_Ast_Node *expression;
        } expr;
    } as;
} *Ruja_Ast;

Ruja_Ast ast_new();
void ast_free(Ruja_Ast ast);

Ruja_Ast ast_new_literal(Ruja_Token* literal_token);
Ruja_Ast ast_new_unary_op(Ruja_Token* unary_token, Ruja_Ast expression);
Ruja_Ast ast_new_binary_op(Ruja_Token* binary_token, Ruja_Ast left_expression, Ruja_Ast right_expression);
Ruja_Ast ast_new_ternary_op(Ruja_Token* tok_question, Ruja_Ast condition, Ruja_Ast true_expression, Ruja_Ast false_expression);
Ruja_Ast ast_new_expression(Ruja_Ast expression);

void ast_dot(Ruja_Ast ast, FILE *file);


#endif // RUJA_AST_H