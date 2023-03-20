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
    AST_NODE_IDENTIFIER,
    AST_NODE_UNARY_OP,
    AST_NODE_BINARY_OP,
    AST_NODE_TERNARY_OP,
    AST_NODE_EXPRESSION,
    
    AST_NODE_STMT_ASSIGN,
    AST_NODE_STMT_TYPED_DECL,
    AST_NODE_STMT_TYPED_DECL_ASSIGN,
    AST_NODE_STMT_INFERRED_DECL_ASSIGN,

    AST_NODE_STMT_IF,
    AST_NODE_STMT_ELIF,
    AST_NODE_STMT_ELSE,
    AST_NODE_STMTS,
} ast_node_type;

typedef struct Ruja_Ast_Node {
    ast_node_type type;
    union {
        struct {
            Ruja_Token* tok_literal;
        } literal;
        struct {
            Ruja_Token* tok_identifier;
        } identifier;
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
        struct {
            Ruja_Token* tok_assign;
            struct Ruja_Ast_Node *identifier;
            struct Ruja_Ast_Node *expression;
        } assign;
        struct {
            Ruja_Token* tok_dtype;
            struct Ruja_Ast_Node *identifier;
        } typed_decl;
        struct {
            Ruja_Token* tok_dtype;
            Ruja_Token* tok_assign;
            struct Ruja_Ast_Node *identifier;
            struct Ruja_Ast_Node *expression;
        } typed_decl_assign;
        struct {
            Ruja_Token* tok_assign;
            struct Ruja_Ast_Node *identifier;
            struct Ruja_Ast_Node *expression;
        } inferred_decl_assign;
        struct {
            Ruja_Token* tok_if;
            struct Ruja_Ast_Node *condition;
            struct Ruja_Ast_Node* body;
            struct Ruja_Ast_Node* next_branch;
        } if_branch;
        struct {
            Ruja_Token* tok_elif;
            struct Ruja_Ast_Node *condition;
            struct Ruja_Ast_Node* body;
            struct Ruja_Ast_Node* next_branch;
        } elif_branch;
        struct {
            Ruja_Token* tok_else;
            struct Ruja_Ast_Node* body;
        } else_branch;
        struct {
            struct Ruja_Ast_Node *statement;
            struct Ruja_Ast_Node *next;
        } stmts;
    } as;
} *Ruja_Ast;

Ruja_Ast ast_new();
void ast_free(Ruja_Ast ast);

Ruja_Ast ast_new_literal(Ruja_Token* literal_token);
Ruja_Ast ast_new_identifier(Ruja_Token* identifier_token);
Ruja_Ast ast_new_unary_op(Ruja_Token* unary_token, Ruja_Ast expression);
Ruja_Ast ast_new_binary_op(Ruja_Token* binary_token, Ruja_Ast left_expression, Ruja_Ast right_expression);
Ruja_Ast ast_new_ternary_op(Ruja_Token* tok_question, Ruja_Token* tok_colon, Ruja_Ast condition, Ruja_Ast true_expression, Ruja_Ast false_expression);
Ruja_Ast ast_new_expression(Ruja_Ast expression);
Ruja_Ast ast_new_assign(Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression);
Ruja_Ast ast_new_typed_decl(Ruja_Token* dtype_token, Ruja_Ast identifier);
Ruja_Ast ast_new_typed_decl_assign(Ruja_Token* dtype_token, Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression);
Ruja_Ast ast_new_inferred_decl_assign(Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression);
Ruja_Ast ast_new_if_stmt(Ruja_Token* if_token, Ruja_Ast condition, Ruja_Ast body, Ruja_Ast else_stmt);
Ruja_Ast ast_new_elif_stmt(Ruja_Token* elif_token, Ruja_Ast condition, Ruja_Ast body, Ruja_Ast else_stmt);
Ruja_Ast ast_new_else_stmt(Ruja_Token* else_token, Ruja_Ast body);
Ruja_Ast ast_new_stmt(Ruja_Ast statement, Ruja_Ast next);

void ast_dot(Ruja_Ast ast, FILE *file);


#endif // RUJA_AST_H