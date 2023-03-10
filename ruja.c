#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "includes/parser.h"
#include "includes/bytecode.h"
#include "includes/stack.h"
#include "includes/vm.h"
#include "includes/ast.h"
#include "includes/compiler.h"

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
    printf("Usage: <path-to>/ruja [OPTIONS] <input-file>\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help\t\tPrint this help message.\n");
    printf("  -v, --version\t\tPrint the version of Ruja.\n");
}

#if 0 // Stack test
int main() {
    Stack* stack = stack_new();


    stack_trace(stack);
    stack_push(stack, MAKE_DOUBLE(-3.14));
    stack_push(stack, MAKE_INT(-12));
    stack_push(stack, MAKE_BOOL(1));
    stack_push(stack, MAKE_CHAR('a'));
    stack_push(stack, MAKE_NIL());
    stack_trace(stack);

    stack_free(stack);
    return 0;
}
#endif

#if 0 // Nan Box test
int main() {
    Word w = MAKE_INT(-1);
    print_word(w, 0);
    printf("\n");
    return 0;
}
#endif

#if 0 // Bytecode test
int main() {
    Ruja_Vm* vm = vm_new();

    size_t index1 = add_constant(vm->bytecode, MAKE_INT(0));
    size_t index2 = add_constant(vm->bytecode, MAKE_INT(8));
    add_opcode(vm->bytecode, OP_CONST, 2);
    add_operand(vm->bytecode, index1, 2);
    add_opcode(vm->bytecode, OP_CONST, 2);
    add_operand(vm->bytecode, index1, 2);
    add_opcode(vm->bytecode, OP_JZ, 2);
    add_operand(vm->bytecode, index2, 2);
    add_opcode(vm->bytecode, OP_NEG, 2);
    add_opcode(vm->bytecode, OP_NOT, 2);
    add_opcode(vm->bytecode, OP_HALT, 2);

    vm_run(vm);

    vm_free(vm);
    return 0;
}
#endif

#if 0 // Lexer test
int main(void) {
    Ruja_Lexer* lexer = lexer_new("tokens.ruja");
    if (lexer != NULL) {
        Ruja_Token* token = NULL;

        do {
            if(token != NULL) token_free(token);
            token = next_token(lexer);
            token_to_string(token);
            if (token->kind == RUJA_TOK_ERR) {
                printf("Error\n");
                break;
            }
        } while (token->kind != RUJA_TOK_EOF);
        token_free(token);

        lexer_free(lexer);
    }

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
                        Ruja_Ast ast = ast_new_expression(NULL);
                        if (ast != NULL) {
                            if (parse(parser, lexer, &ast)) {
                                ast_dot(ast, stdout);
                            }

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
    Ruja_Ast number1 = ast_new_literal(MAKE_DOUBLE(1));
    Ruja_Ast number2 = ast_new_literal(MAKE_DOUBLE(2));
    Ruja_Ast number3 = ast_new_literal(MAKE_DOUBLE(3));

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

#if 0 // Compiler test
int main(void) {
    Ruja_Compiler* compiler = compiler_new();
    Ruja_Vm* vm = vm_new();

    if (compile(compiler, "input.ruja", vm->bytecode) != RUJA_COMPILER_ERROR) {
        // disassemble(vm->bytecode, "code");
        vm_run(vm);
    }

    vm_free(vm);
    compiler_free(compiler);
    return 0;
}
#endif