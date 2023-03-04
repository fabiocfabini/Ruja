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
            return "not";
        case AST_UNARY_OP_NEG:
            return "-";
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
        case AST_BINARY_OP_NE:
            return "!=";
        case AST_BINARY_OP_LT:
            return "<";
        case AST_BINARY_OP_LE:
            return "<=";
        case AST_BINARY_OP_GT:
            return ">";
        case AST_BINARY_OP_GE:
            return ">=";
        default:
            return "unknown";
    }
}

static size_t increment(size_t* id) {
    return ++(*id);
}

static void dot_node(FILE* file, size_t id, const char* label, const char* color, const char* style) {
    fprintf(file, "    %zu [label=\"%s\", fillcolor=\"%s\", style=\"%s\"];\n", id, label, color?color:"black", style?style:"");
}

static void dot_node_word(FILE* file, size_t id, Word word, const char* color, const char* style) {
    fprintf(file, "    %zu [label=\"%lf\", fillcolor=\"%s\", style=\"%s\"];\n", id, word, color?color:"black", style?style:"");
}

static void dot_arrow(FILE* file, size_t from, size_t to, const char* label) {
    fprintf(file, "    %zu -> %zu [label=\"%s\"];\n", from, to, label);
}

static void ast_dot_internal(Ruja_Ast ast, FILE* file, size_t* id) {
// Dark colors
#define DARK_BLUE "#0000CC"
#define DARK_GREEN "#00CC00"
#define DARK_RED "#CC0000"

// Light colors
#define EXPRESSION_COLOR "#CCE6FF"
#define LITERAL_COLOR "#CCFFCC"
#define ARITHMETIC_COLOR "#FFCCCC"
    if (ast == NULL) return;

    size_t root_id = *id;
    switch (ast->type) {
        case AST_NODE_EMPTY:
            dot_node(file, root_id, "Empty", DARK_RED, "filled");
            break;
        case AST_NODE_NUMBER:
            dot_node(file, root_id, "Number", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "word");
            dot_node_word(file, *id, ast->as.number.word, LITERAL_COLOR, "filled");
            break;
        case AST_NODE_UNARY_OP:
            dot_node(file, root_id, "UnaryOp", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, ast_unary_op_type_to_string(ast->as.unary_op.type), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.unary_op.expression, file, id);
            break;
        case AST_NODE_BINARY_OP:
            dot_node(file, root_id, "BinaryOp", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "left_expression");
            ast_dot_internal(ast->as.binary_op.left_expression, file, id);
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, ast_binary_op_type_to_string(ast->as.binary_op.type), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "right_expression");
            ast_dot_internal(ast->as.binary_op.right_expression, file, id);
            break;
        case AST_NODE_TERNARY_OP:
            dot_node(file, root_id, "TernaryOp", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "condition");
            ast_dot_internal(ast->as.ternary_op.condition, file, id);
            dot_arrow(file, root_id, increment(id), "true_expression");
            ast_dot_internal(ast->as.ternary_op.true_expression, file, id);
            dot_arrow(file, root_id, increment(id), "false_expression");
            ast_dot_internal(ast->as.ternary_op.false_expression, file, id);
            break;
        case AST_NODE_EXPRESSION:
            dot_node(file, root_id, "Expression", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.expr.expression, file, id);
            break;
    }

// Dark colors
#undef DARK_BLUE
#undef DARK_GREEN
#undef DARK_RED

// Light colors
#undef EXPRESSION_COLOR
#undef LITERAL_COLOR
#undef ARITHMETIC_COLOR
}

void ast_dot(Ruja_Ast ast, FILE *file) {
    fprintf(file, "digraph ast {\n");
    fprintf(file, "    graph [rankdir=LR];\n");
    fprintf(file, "    node [shape=box];\n");
    size_t id = 0;
    ast_dot_internal(ast, file, &id);
    fprintf(file, "}\n");
}
