#include <stdio.h>
#include <stdlib.h>

#include "../includes/ast.h"
#include "../includes/bytecode.h"

Ruja_Ast ast_new() {
    Ruja_Ast ast = malloc(sizeof(struct Ruja_Ast_Node));
    if (ast == NULL) {
        fprintf(stderr, "Failed to allocate memory for ast\n");
        return NULL;
    }

    ast->type = AST_NODE_EMPTY;
    return ast;
}

void ast_free(Ruja_Ast ast) {
    if (ast == NULL) return;

    switch (ast->type) {
        case AST_NODE_EMPTY:
            break;
        case AST_NODE_NUMBER:
            break;
        case AST_NODE_UNARY_OP:
            ast_free(ast->as.unary_op.expression);
            break;
        case AST_NODE_BINARY_OP:
            ast_free(ast->as.binary_op.left_expression);
            ast_free(ast->as.binary_op.right_expression);
            break;
        case AST_NODE_TERNARY_OP:
            ast_free(ast->as.ternary_op.condition);
            ast_free(ast->as.ternary_op.true_expression);
            ast_free(ast->as.ternary_op.false_expression);
            break;
        case AST_NODE_EXPRESSION:
            ast_free(ast->as.expr.expression);
            break;
    }

    free(ast);
}

Ruja_Ast ast_new_number(Word word) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_NUMBER;
    ast->as.number.word = word;

    return ast;
}

Ruja_Ast ast_new_unary_op(ast_unary_op_type type, Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_UNARY_OP;
    ast->as.unary_op.type = type;
    ast->as.unary_op.expression = expression;

    return ast;
}

Ruja_Ast ast_new_binary_op(ast_binary_op_type type, Ruja_Ast left_expression, Ruja_Ast right_expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_BINARY_OP;
    ast->as.binary_op.type = type;
    ast->as.binary_op.left_expression = left_expression;
    ast->as.binary_op.right_expression = right_expression;

    return ast;
}

Ruja_Ast ast_new_ternary_op(Ruja_Ast condition, Ruja_Ast true_expression, Ruja_Ast false_expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_TERNARY_OP;
    ast->as.ternary_op.condition = condition;
    ast->as.ternary_op.true_expression = true_expression;
    ast->as.ternary_op.false_expression = false_expression;

    return ast;
}

Ruja_Ast ast_new_expression(Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_EXPRESSION;
    ast->as.expr.expression = expression;

    return ast;
}

const char *ast_unary_op_type_to_string(ast_unary_op_type type) {
    switch (type) {
        case AST_UNARY_OP_NOT:
            return "-";
        case AST_UNARY_OP_NEG:
            return "not";
        default:
            return "unknown";
    }
}

const char *ast_binary_op_type_to_string(ast_binary_op_type type) {
    switch (type) {
        case AST_BINARY_OP_ADD:
            return "+";
        case AST_BINARY_OP_SUB:
            return "-";
        case AST_BINARY_OP_MUL:
            return "*";
        case AST_BINARY_OP_DIV:
            return "/";
        case AST_BINARY_OP_AND:
            return "and";
        case AST_BINARY_OP_OR:
            return "or";
        case AST_BINARY_OP_EQ:
            return "==";
        case AST_BINARY_OP_NEQ:
            return "!=";
        case AST_BINARY_OP_LT:
            return "<";
        case AST_BINARY_OP_LTE:
            return "<=";
        case AST_BINARY_OP_GT:
            return ">";
        case AST_BINARY_OP_GTE:
            return ">=";
        default:
            return "unknown";
    }
}

static size_t increment(size_t* id) {
    return ++(*id);
}

static void ast_dot_internal(Ruja_Ast ast, FILE* file, size_t* id) {
    if (ast == NULL) return;

    size_t root_id = *id;
    switch (ast->type) {
        case AST_NODE_EMPTY:
            fprintf(file, "    %zu [label=\"Empty\"];\n", root_id);
            break;
        case AST_NODE_NUMBER:
            fprintf(file, "    %zu [label=\"Number\"];\n", root_id);
            fprintf(file, "    %zu -> %zu [label=\"word\"];\n", root_id, increment(id));
            fprintf(file, "    %zu [label=\"%lf\"];\n", *id, ast->as.number.word);
            break;
        case AST_NODE_UNARY_OP:
            fprintf(file, "    %zu [label=\"UnaryOp\"];\n", root_id);
            fprintf(file, "    %zu -> %zu [label=\"type\"];\n", root_id, increment(id));
            fprintf(file, "    %zu [label=\"%s\"];\n", *id, ast_unary_op_type_to_string(ast->as.unary_op.type));
            fprintf(file, "    %zu -> %zu [label=\"expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.unary_op.expression, file, id);
            break;
        case AST_NODE_BINARY_OP:
            fprintf(file, "    %zu [label=\"BinaryOp\"];\n", root_id);
            fprintf(file, "    %zu -> %zu [label=\"left_expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.binary_op.left_expression, file, id);
            fprintf(file, "    %zu -> %zu [label=\"type\"];\n", root_id, increment(id));
            fprintf(file, "    %zu [label=\"%s\"];\n", *id, ast_binary_op_type_to_string(ast->as.binary_op.type));
            fprintf(file, "    %zu -> %zu [label=\"right_expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.binary_op.right_expression, file, id);
            break;
        case AST_NODE_TERNARY_OP:
            fprintf(file, "    %zu [label=\"TernaryOp\"];\n", root_id);
            fprintf(file, "    %zu -> %zu [label=\"condition\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.ternary_op.condition, file, id);
            fprintf(file, "    %zu -> %zu [label=\"true_expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.ternary_op.true_expression, file, id);
            fprintf(file, "    %zu -> %zu [label=\"false_expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.ternary_op.false_expression, file, id);
            break;
        case AST_NODE_EXPRESSION:
            fprintf(file, "    %zu [label=\"Expression\"];\n", root_id);
            fprintf(file, "    %zu -> %zu [label=\"expression\"];\n", root_id, increment(id));
            ast_dot_internal(ast->as.expr.expression, file, id);
            break;
    }
}

void ast_dot(Ruja_Ast ast, FILE *file) {
    fprintf(file, "digraph ast {\n");
    fprintf(file, "    layout=sfdp;\n");
    fprintf(file, "    node [shape=box];\n");
    size_t id = 0;
    ast_dot_internal(ast, file, &id);
    fprintf(file, "}\n");
}
