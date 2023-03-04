#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "includes/parser.h"
#include "includes/bytecode.h"
#include "includes/stack.h"
#include "includes/vm.h"
#include "includes/ast.h"

void shift_agrs(int* argc, char*** argv) {
    (*argc)--;
    (*argv)++;
}

bool endswith(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) {
        return false;
    }
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void usage() {
    printf("Usage: <path-to>/ruja <input-file> [OPTIONS]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help\t\tPrint this help message.\n");
    printf("  -v, --version\t\tPrint the version of Ruja.\n");
}

#if 0 // Stack test
int main() {
    Stack* stack = stack_new();


    stack_trace(stack);
    stack_push(stack, 1.1);
    stack_push(stack, 1.2);
    stack_push(stack, 1.3);
    stack_push(stack, 1.4);
    stack_push(stack, 1.5);
    stack_trace(stack);

    stack_free(stack);
    return 0;
}
#endif

#if 0 // Bytecode test
int main() {
    Ruja_Vm* vm = vm_new();

    size_t index1 = add_constant(vm->bytecode, 1);
    size_t index2 = add_constant(vm->bytecode, 0);
    add_opcode(vm->bytecode, OP_CONST, 1);
    add_opcode(vm->bytecode, (uint8_t) index1, 1);
    add_opcode(vm->bytecode, OP_NEG, 1);
    add_opcode(vm->bytecode, OP_CONST, 1);
    add_opcode(vm->bytecode, (uint8_t) index2, 1);
    add_opcode(vm->bytecode, OP_LT, 1);
    add_opcode(vm->bytecode, OP_HALT, 2);

    vm_run(vm);

    vm_free(vm);
    return 0;
}
#endif

#if 0 // Parser test
int main2(int argc, char** argv) {
    if (argc < 2) {
        usage(); return 1;
    } else {
        while (argc > 1) {
            shift_agrs(&argc, &argv);
            if (strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
                usage(); return 0;
            } else if (strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0) {
                printf("Ruja 0.0.1\n"); return 0;
            } else if (endswith(*argv, ".ruja")) {
                Ruja_Lexer* lexer = lexer_new(*argv);
                if (lexer != NULL) {
                    Ruja_Parser* parser = parser_new();
                    if (parser != NULL) {
                        parse(parser, lexer);

                        parser_free(parser);
                    }
                    lexer_free(lexer);
                }
            } else {
                printf("Unknown option '%s'.\n", *argv);
                usage(); return 1;
            }
        }
    }

    return 0;
}
#endif

#if 1 // AST test
int main() {
    Ruja_Ast ast = ast_new();

    // 1 + 2 * 3

    // Create the root node
    ast->type = AST_NODE_EXPRESSION;
    ast->as.expr.expression = ast_new();

    // Create the left node
    ast->as.expr.expression->type = AST_NODE_BINARY_OP;
    ast->as.expr.expression->as.binary_op.type = AST_BINARY_OP_ADD;
    ast->as.expr.expression->as.binary_op.left_expression = ast_new();
    ast->as.expr.expression->as.binary_op.left_expression->type = AST_NODE_NUMBER;
    ast->as.expr.expression->as.binary_op.left_expression->as.number.word = 1;

    // Create the right node
    ast->as.expr.expression->as.binary_op.right_expression = ast_new();
    ast->as.expr.expression->as.binary_op.right_expression->type = AST_NODE_BINARY_OP;
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.type = AST_BINARY_OP_MUL;
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.left_expression = ast_new();
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.left_expression->type = AST_NODE_NUMBER;
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.left_expression->as.number.word = 2;
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.right_expression = ast_new();
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.right_expression->type = AST_NODE_NUMBER;
    ast->as.expr.expression->as.binary_op.right_expression->as.binary_op.right_expression->as.number.word = 3;

    ast_dot(ast, stdout);
    ast_free(ast);
    return 0;
}
#endif