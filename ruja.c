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

#if 1 // Parser test
int main(int argc, char** argv) {
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
                        Ruja_Ast ast = ast_new();
                        if (ast != NULL) {
                            parse(parser, lexer, &ast);

                            ast_dot(ast, stdout);
                            ast_free(ast);
                        }
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

#if 0 // AST test
int main() {
    // -(1 + 2 * 3)

    // Crete the numbers
    Ruja_Ast number1 = ast_new_number(1);
    Ruja_Ast number2 = ast_new_number(2);
    Ruja_Ast number3 = ast_new_number(3);

    // Create the multiplication
    Ruja_Ast multiplication = ast_new_binary_op(AST_BINARY_OP_MUL, number1, number3);

    // Create the addition
    Ruja_Ast addition = ast_new_binary_op(AST_BINARY_OP_ADD, number2, multiplication);

    // Create the negation
    Ruja_Ast negation = ast_new_unary_op(AST_UNARY_OP_NEG, addition);

    // Create the Expression
    Ruja_Ast expression = ast_new_expression(negation);

    ast_dot(expression, stdout);
    ast_free(expression);
    return 0;
}
#endif