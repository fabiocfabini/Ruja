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
        case AST_NODE_LITERAL:
            token_free(ast->as.literal.tok_literal);
            break;
        case AST_NODE_IDENTIFIER:
            token_free(ast->as.identifier.tok_identifier);
            break;
        case AST_NODE_UNARY_OP:
            token_free(ast->as.unary_op.tok_unary);
            ast_free(ast->as.unary_op.expression);
            break;
        case AST_NODE_BINARY_OP:
            token_free(ast->as.binary_op.tok_binary);
            ast_free(ast->as.binary_op.left_expression);
            ast_free(ast->as.binary_op.right_expression);
            break;
        case AST_NODE_TERNARY_OP:
            token_free(ast->as.ternary_op.tok_ternary.tok_question);
            token_free(ast->as.ternary_op.tok_ternary.tok_colon);
            ast_free(ast->as.ternary_op.condition);
            ast_free(ast->as.ternary_op.true_expression);
            ast_free(ast->as.ternary_op.false_expression);
            break;
        case AST_NODE_EXPRESSION:
            ast_free(ast->as.expr.expression);
            break;
        case AST_NODE_STMT_ASSIGN:
            token_free(ast->as.assign.tok_assign);
            ast_free(ast->as.assign.identifier);
            ast_free(ast->as.assign.expression);
            break;
        case AST_NODE_STMT_TYPED_DECL:
            token_free(ast->as.typed_decl.tok_dtype);
            ast_free(ast->as.typed_decl.identifier);
            break;
        case AST_NODE_STMT_TYPED_DECL_ASSIGN:
            token_free(ast->as.typed_decl_assign.tok_dtype);
            token_free(ast->as.typed_decl_assign.tok_assign);
            ast_free(ast->as.typed_decl_assign.identifier);
            ast_free(ast->as.typed_decl_assign.expression);
            break;
        case AST_NODE_STMT_INFERRED_DECL_ASSIGN:
            token_free(ast->as.inferred_decl_assign.tok_assign);
            ast_free(ast->as.inferred_decl_assign.identifier);
            ast_free(ast->as.inferred_decl_assign.expression);
            break;
        case AST_NODE_STMT_IF:
            token_free(ast->as.if_branch.tok_if);
            ast_free(ast->as.if_branch.condition);
            ast_free(ast->as.if_branch.body);
            ast_free(ast->as.if_branch.next_branch);
            break;
        case AST_NODE_STMT_ELIF:
            token_free(ast->as.elif_branch.tok_elif);
            ast_free(ast->as.elif_branch.condition);
            ast_free(ast->as.elif_branch.body);
            ast_free(ast->as.elif_branch.next_branch);
            break;
        case AST_NODE_STMT_ELSE:
            token_free(ast->as.else_branch.tok_else);
            ast_free(ast->as.else_branch.body);
            break;
        case AST_NODE_RANGED_ITER:
            ast_free(ast->as.ranged_iter.start_expr);
            ast_free(ast->as.ranged_iter.end_expr);
            ast_free(ast->as.ranged_iter.step_expr);
            break;
        case AST_NODE_STMT_FOR:
            token_free(ast->as.for_loop.tok_for);
            token_free(ast->as.for_loop.tok_in);
            ast_free(ast->as.for_loop.identifier);
            ast_free(ast->as.for_loop.iter);
            ast_free(ast->as.for_loop.body);
            break;
        case AST_NODE_STMT_WHILE:
            token_free(ast->as.while_loop.tok_while);
            ast_free(ast->as.while_loop.condition);
            ast_free(ast->as.while_loop.body);
            break;
        case AST_NODE_STMT_STRUCT_MEMBER:
            token_free(ast->as.struct_member.tok_dtype);
            ast_free(ast->as.struct_member.identifier);
            ast_free(ast->as.struct_member.next_member);
            break;
        case AST_NODE_STMT_STRUCT_DEF:
            token_free(ast->as.struct_def.tok_struct);
            ast_free(ast->as.struct_def.identifier);
            ast_free(ast->as.struct_def.members);
            break;
        case AST_NODE_STMTS:
            ast_free(ast->as.stmts.statement);
            ast_free(ast->as.stmts.next);
            break;
    }

    free(ast);
}

Ruja_Ast ast_new_literal(Ruja_Token* literal_token) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_LITERAL;
    ast->as.literal.tok_literal = literal_token;

    literal_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_identifier(Ruja_Token* identifier_token) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_IDENTIFIER;
    ast->as.identifier.tok_identifier = identifier_token;

    identifier_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_unary_op(Ruja_Token* unary_token, Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_UNARY_OP;
    ast->as.unary_op.tok_unary = unary_token;
    ast->as.unary_op.expression = expression;

    unary_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_binary_op(Ruja_Token* binary_token, Ruja_Ast left_expression, Ruja_Ast right_expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_BINARY_OP;
    ast->as.binary_op.tok_binary = binary_token;
    ast->as.binary_op.left_expression = left_expression;
    ast->as.binary_op.right_expression = right_expression;

    binary_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_ternary_op(Ruja_Token* tok_question, Ruja_Token* tok_colon, Ruja_Ast condition, Ruja_Ast true_expression, Ruja_Ast false_expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_TERNARY_OP;
    ast->as.ternary_op.tok_ternary.tok_question = tok_question;
    ast->as.ternary_op.tok_ternary.tok_colon = tok_colon;
    ast->as.ternary_op.condition = condition;
    ast->as.ternary_op.true_expression = true_expression;
    ast->as.ternary_op.false_expression = false_expression;

    tok_question->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_expression(Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_EXPRESSION;
    ast->as.expr.expression = expression;

    return ast;
}

Ruja_Ast ast_new_assign(Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_ASSIGN;
    ast->as.assign.tok_assign = assign_token;
    ast->as.assign.identifier = identifier;
    ast->as.assign.expression = expression;

    assign_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_typed_decl(Ruja_Token* dtype_token, Ruja_Ast identifier) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_TYPED_DECL;
    ast->as.typed_decl.tok_dtype = dtype_token;
    ast->as.typed_decl.identifier = identifier;

    dtype_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_typed_decl_assign(Ruja_Token* dtype_token, Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_TYPED_DECL_ASSIGN;
    ast->as.typed_decl_assign.tok_dtype = dtype_token;
    ast->as.typed_decl_assign.tok_assign = assign_token;
    ast->as.typed_decl_assign.identifier = identifier;
    ast->as.typed_decl_assign.expression = expression;

    dtype_token->in_ast = true;
    assign_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_inferred_decl_assign(Ruja_Token* assign_token, Ruja_Ast identifier, Ruja_Ast expression) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_INFERRED_DECL_ASSIGN;
    ast->as.inferred_decl_assign.tok_assign = assign_token;
    ast->as.inferred_decl_assign.identifier = identifier;
    ast->as.inferred_decl_assign.expression = expression;

    assign_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_if_stmt(Ruja_Token* if_token, Ruja_Ast condition, Ruja_Ast body, Ruja_Ast next_branch) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_IF;
    ast->as.if_branch.tok_if = if_token;
    ast->as.if_branch.condition = condition;
    ast->as.if_branch.body = body;
    ast->as.if_branch.next_branch = next_branch;

    if_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_elif_stmt(Ruja_Token* elif_token, Ruja_Ast condition, Ruja_Ast body, Ruja_Ast else_stmt) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_ELIF;
    ast->as.elif_branch.tok_elif = elif_token;
    ast->as.elif_branch.condition = condition;
    ast->as.elif_branch.body = body;
    ast->as.elif_branch.next_branch = else_stmt;

    elif_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_else_stmt(Ruja_Token* else_token, Ruja_Ast body) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_ELSE;
    ast->as.else_branch.tok_else = else_token;
    ast->as.else_branch.body = body;

    else_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_ranged_iter(Ruja_Ast start_expr, Ruja_Ast end_expr, Ruja_Ast step_expr) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_RANGED_ITER;
    ast->as.ranged_iter.start_expr = start_expr;
    ast->as.ranged_iter.end_expr = end_expr;
    ast->as.ranged_iter.step_expr = step_expr;

    return ast;
}

Ruja_Ast ast_new_for_loop(Ruja_Token* for_token, Ruja_Ast identifier, Ruja_Ast iter, Ruja_Ast body) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_FOR;
    ast->as.for_loop.tok_for = for_token;
    ast->as.for_loop.tok_in = NULL;
    ast->as.for_loop.identifier = identifier;
    ast->as.for_loop.iter = iter;
    ast->as.for_loop.body = body;

    for_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_while_loop(Ruja_Token* while_token, Ruja_Ast condition, Ruja_Ast body) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_WHILE;
    ast->as.while_loop.tok_while = while_token;
    ast->as.while_loop.condition = condition;
    ast->as.while_loop.body = body;

    while_token->in_ast = true;
    return ast;
}

Ruja_Ast ast_new_struct_members(Ruja_Ast identifier_token, Ruja_Ast next_member) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_STRUCT_MEMBER;
    ast->as.struct_member.tok_dtype = NULL;
    ast->as.struct_member.identifier = identifier_token;
    ast->as.struct_member.next_member = next_member;

    return ast;
}

Ruja_Ast ast_new_struct_def(Ruja_Token* struct_token, Ruja_Ast identifier_token, Ruja_Ast members) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMT_STRUCT_DEF;
    ast->as.struct_def.tok_struct = struct_token;
    ast->as.struct_def.identifier = identifier_token;
    ast->as.struct_def.members = members;

    struct_token->in_ast = true;
    return ast;
}



Ruja_Ast ast_new_stmt(Ruja_Ast statement, Ruja_Ast next) {
    Ruja_Ast ast = ast_new();
    if (ast == NULL) return NULL;

    ast->type = AST_NODE_STMTS;
    ast->as.stmts.statement = statement;
    ast->as.stmts.next = next;

    return ast;
}

static const char* assign_to_string(Ruja_Token_Kind kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (kind) {
        case RUJA_TOK_ASSIGN:
            return "=";
        case RUJA_TOK_ADD_EQ:
            return "+=";
        case RUJA_TOK_SUB_EQ:
            return "-=";
        case RUJA_TOK_MUL_EQ:
            return "*=";
        case RUJA_TOK_DIV_EQ:
            return "/=";
        default:
            return "unknown";
    }
#pragma GCC diagnostic pop
}

static const char* type_to_string(Ruja_Token_Kind kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (kind) {
        case RUJA_TOK_TYPE_BOOL:
            return "bool";
        case RUJA_TOK_TYPE_CHAR:
            return "char";
        case RUJA_TOK_TYPE_I32:
            return "int";
        case RUJA_TOK_TYPE_F64:
            return "float";
        case RUJA_TOK_TYPE_STRING:
            return "string";
        default:
            return "unknown";
    }
#pragma GCC diagnostic pop
}

/**
 * @brief Convert an ast_unary_op_type to a string
 * 
 * @param type 
 * @return const char* 
 */
const char *unary_token_kind_to_string(Ruja_Token_Kind kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (kind) {
        case RUJA_TOK_NOT:
            return "not";
        case RUJA_TOK_SUB:
            return "-";
        default:
            return "unknown";
    }
#pragma GCC diagnostic pop
}

/**
 * @brief Convert an ast_binary_op_type to a string
 * 
 * @param type 
 * @return const char* 
 */
const char *binary_token_kind_to_string(Ruja_Token_Kind kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (kind) {
        case RUJA_TOK_ADD:
            return "+";
        case RUJA_TOK_SUB:
            return "-";
        case RUJA_TOK_MUL:
            return "*";
        case RUJA_TOK_DIV:
            return "/";
        case RUJA_TOK_AND:
            return "and";
        case RUJA_TOK_OR:
            return "or";
        case RUJA_TOK_EQ:
            return "==";
        case RUJA_TOK_NE:
            return "!=";
        case RUJA_TOK_LT:
            return "<";
        case RUJA_TOK_LE:
            return "<=";
        case RUJA_TOK_GT:
            return ">";
        case RUJA_TOK_GE:
            return ">=";
        default:
            return "unknown";
    }
#pragma GCC diagnostic pop
}

/**
 * @brief Increment an id and return the new value 
 * 
 * 
 * @param type 
 * @return size_t The new value of the id
 */
static size_t increment(size_t* id) {
    return ++(*id);
}

/**
 * @brief Print an ast node to a file in dot format
 * 
 * @param file The file pointer
 * @param id The id of the node
 * @param label The label of the node
 * @param color The color of the node
 * @param style The style of the node
 */
static void dot_node(FILE* file, size_t id, const char* label, const char* color, const char* style) {
    fprintf(file, "    %zu [label=\"%s\", fillcolor=\"%s\", style=\"%s\"];\n", id, label, color?color:"black", style?style:"");
}

static void print_token_word(FILE* file, Ruja_Token* token) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (token->kind) {
        case RUJA_TOK_INT: fprintf(file, "%ld", strtol(token->start, NULL, 10)); break;
        case RUJA_TOK_FLOAT: fprintf(file, "%lf", strtod(token->start, NULL)); break;
        case RUJA_TOK_CHAR: fprintf(file, "%c", *(token->start)); break;
        case RUJA_TOK_TRUE: fprintf(file, "%.*s", (int) token->length, token->start); break;
        case RUJA_TOK_FALSE: fprintf(file, "%.*s", (int) token->length, token->start); break;
        case RUJA_TOK_ID: fprintf(file, "%.*s", (int) token->length, token->start); break;
        case RUJA_TOK_STRING: fprintf(file, "\\\"%.*s\\\"", (int) token->length, token->start); break;
        case RUJA_TOK_NIL: fprintf(file, "%.*s", (int) token->length, token->start); break;
        default: fprintf(file, "UNKNOWN");
    }
#pragma GCC diagnostic pop
}

/**
 * @brief Print an ast word node to a file in dot format
 * 
 * @param file The file pointer
 * @param id The id of the node
 * @param word The word of the node
 * @param color The color of the node
 * @param style The style of the node
 */
static void dot_node_word(FILE* file, size_t id, Ruja_Token* token, const char* color, const char* style) {
    fprintf(file, "    %zu [label=\"", id);
    print_token_word(file, token);
    fprintf(file, "\", fillcolor=\"%s\", style=\"%s\"];\n", color?color:"black", style?style:"");
}

/**
 * @brief Print an ast arrow to a file in dot format
 * 
 * @param file The file pointer
 * @param from The id of the node the arrow is coming from
 * @param to The id of the node the arrow is going to
 * @param label The label of the arrow
 */
static void dot_arrow(FILE* file, size_t from, size_t to, const char* label) {
    fprintf(file, "    %zu -> %zu [label=\"%s\"];\n", from, to, label);
}

/**
 * @brief Internal recursive function to print an ast to a file in dot format
 * 
 * @param ast The ast to print
 * @param file The file pointer
 */
static void ast_dot_internal(Ruja_Ast ast, FILE* file, size_t* id) {
// Dark colors
#define DARK_BLUE "#0000CC"
#define DARK_GREEN "#00CC00"
#define DARK_RED "#CC0000"

// Light colors
#define LOOP_COLOR "#F391F6"
#define BRANCH_COLOR "#91F6F6"
#define STATEMENT_COLOR "#F6CD91"
#define EXPRESSION_COLOR "#CCE6FF"
#define LITERAL_COLOR "#CCFFCC"
#define ARITHMETIC_COLOR "#FFCCCC"
#define IDENTIFIER_COLOR "#FFFFCC"
    if (ast == NULL) return;

    size_t root_id = *id;
    switch (ast->type) {
        case AST_NODE_EMPTY:
            dot_node(file, root_id, "Empty", DARK_RED, "filled");
            break;
        case AST_NODE_LITERAL:
            dot_node(file, root_id, "Literal", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "value");
            dot_node_word(file, *id, ast->as.literal.tok_literal, LITERAL_COLOR, "filled");
            break;
        case AST_NODE_IDENTIFIER:
            dot_node(file, root_id, "Identifier", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "name");
            dot_node_word(file, *id, ast->as.literal.tok_literal, IDENTIFIER_COLOR, "filled");
            break;
        case AST_NODE_UNARY_OP:
            dot_node(file, root_id, "UnaryOp", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, unary_token_kind_to_string(ast->as.unary_op.tok_unary->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.unary_op.expression, file, id);
            break;
        case AST_NODE_BINARY_OP:
            dot_node(file, root_id, "BinaryOp", EXPRESSION_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "left_expression");
            ast_dot_internal(ast->as.binary_op.left_expression, file, id);
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, binary_token_kind_to_string(ast->as.binary_op.tok_binary->kind), ARITHMETIC_COLOR, "filled");
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
        case AST_NODE_STMT_ASSIGN:
            dot_node(file, root_id, "Assignment", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.assign.identifier, file, id);
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.assign.expression, file, id);
            break;
        case AST_NODE_STMT_TYPED_DECL:
            dot_node(file, root_id, "TypedDeclaration", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, type_to_string(ast->as.typed_decl.tok_dtype->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.typed_decl.identifier, file, id);
            break;
        case AST_NODE_STMT_TYPED_DECL_ASSIGN:
            dot_node(file, root_id, "TypedDeclarationAssignment", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, type_to_string(ast->as.typed_decl_assign.tok_dtype->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "assign");
            dot_node(file, *id, assign_to_string(ast->as.typed_decl_assign.tok_assign->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.typed_decl_assign.identifier, file, id);
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.typed_decl_assign.expression, file, id);
            break;
        case AST_NODE_STMT_INFERRED_DECL_ASSIGN:
            dot_node(file, root_id, "InferredDeclarationAssignment", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.inferred_decl_assign.identifier, file, id);
            dot_arrow(file, root_id, increment(id), "assign");
            dot_node(file, *id, assign_to_string(ast->as.inferred_decl_assign.tok_assign->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "expression");
            ast_dot_internal(ast->as.inferred_decl_assign.expression, file, id);
            break;
        case AST_NODE_STMT_IF:
            dot_node(file, root_id, "IfBranch", BRANCH_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "condition");
            ast_dot_internal(ast->as.if_branch.condition, file, id);
            dot_arrow(file, root_id, increment(id), "body");
            ast_dot_internal(ast->as.if_branch.body, file, id);
            if (ast->as.if_branch.next_branch != NULL) {
                dot_arrow(file, root_id, increment(id), "next");
                ast_dot_internal(ast->as.if_branch.next_branch, file, id);
            }
            break;
        case AST_NODE_STMT_ELIF:
            dot_node(file, root_id, "ElifBranch", BRANCH_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "condition");
            ast_dot_internal(ast->as.elif_branch.condition, file, id);
            dot_arrow(file, root_id, increment(id), "body");
            ast_dot_internal(ast->as.elif_branch.body, file, id);
            if (ast->as.elif_branch.next_branch != NULL) {
                dot_arrow(file, root_id, increment(id), "next");
                ast_dot_internal(ast->as.elif_branch.next_branch, file, id);
            }
            break;
        case AST_NODE_STMT_ELSE:
            dot_node(file, root_id, "ElseBranch", BRANCH_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "body");
            ast_dot_internal(ast->as.else_branch.body, file, id);
            break;
        case AST_NODE_RANGED_ITER:
            dot_node(file, root_id, "RangedIteration", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "start");
            ast_dot_internal(ast->as.ranged_iter.start_expr, file, id);
            dot_arrow(file, root_id, increment(id), "end");
            ast_dot_internal(ast->as.ranged_iter.end_expr, file, id);
            if (ast->as.ranged_iter.step_expr != NULL) {
                dot_arrow(file, root_id, increment(id), "step");
                ast_dot_internal(ast->as.ranged_iter.step_expr, file, id);
            }
            break;
        case AST_NODE_STMT_FOR:
            dot_node(file, root_id, "ForLoop", LOOP_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.for_loop.identifier, file, id);
            dot_arrow(file, root_id, increment(id), "iterable");
            ast_dot_internal(ast->as.for_loop.iter, file, id);
            dot_arrow(file, root_id, increment(id), "body");
            ast_dot_internal(ast->as.for_loop.body, file, id);
            break;
        case AST_NODE_STMT_WHILE:
            dot_node(file, root_id, "WhileLoop", LOOP_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "condition");
            ast_dot_internal(ast->as.while_loop.condition, file, id);
            dot_arrow(file, root_id, increment(id), "body");
            ast_dot_internal(ast->as.while_loop.body, file, id);
            break;
        case AST_NODE_STMT_STRUCT_MEMBER:
            dot_node(file, root_id, "StructMember", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "type");
            dot_node(file, *id, type_to_string(ast->as.struct_member.tok_dtype->kind), ARITHMETIC_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.struct_member.identifier, file, id);
            if (ast->as.struct_member.next_member != NULL) {
                dot_arrow(file, root_id, increment(id), "next");
                ast_dot_internal(ast->as.struct_member.next_member, file, id);
            }
            break;
        case AST_NODE_STMT_STRUCT_DEF:
            dot_node(file, root_id, "StructDefinition", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "identifier");
            ast_dot_internal(ast->as.struct_def.identifier, file, id);
            dot_arrow(file, root_id, increment(id), "members");
            ast_dot_internal(ast->as.struct_def.members, file, id);
            break;
        case AST_NODE_STMTS:
            dot_node(file, root_id, "Statements", STATEMENT_COLOR, "filled");
            dot_arrow(file, root_id, increment(id), "statement");
            ast_dot_internal(ast->as.stmts.statement, file, id);
            if (ast->as.stmts.next != NULL) {
                dot_arrow(file, root_id, increment(id), "next");
                ast_dot_internal(ast->as.stmts.next, file, id);
            }
            break;
    }

// Dark colors
#undef DARK_BLUE
#undef DARK_GREEN
#undef DARK_RED

// Light colors
#undef LOOP_COLOR
#undef BRANCH_COLOR
#undef STATEMENT_COLOR
#undef EXPRESSION_COLOR
#undef LITERAL_COLOR
#undef ARITHMETIC_COLOR
#undef IDENTIFIER_COLOR
}

void ast_dot(Ruja_Ast ast, FILE *file) {
    fprintf(file, "digraph ast {\n");
    fprintf(file, "    graph [rankdir=LR];\n");
    fprintf(file, "    node [shape=box];\n");
    size_t id = 0;
    ast_dot_internal(ast, file, &id);
    fprintf(file, "}\n");
}
