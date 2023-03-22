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

#define STACK_TEST 0
#define NAN_BOX_TEST 0
#define BYTECODE_TEST 0
#define LEXER_TEST 0
#define PARSER_TEST 1
#define AST_TEST 0
#define COMPILER_TEST 0

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

#if STACK_TEST
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

#if NAN_BOX_TEST
int main() {
    Word w = MAKE_STRING("Hello, World!\n", 14);
    print_word(stdout, w, 0);
    object_free(AS_OBJECT(w));
    return 0;
}
#endif

#if BYTECODE_TEST
int main() {
    Ruja_Vm* vm = vm_new();

    Word w1 = MAKE_STRING("Hello,", 6);
    Word w2 = MAKE_STRING(" World", 6);
    Word w3 = MAKE_STRING(" from Portugal!", 15);

    size_t index1 = add_constant(vm->bytecode, w1);
    size_t index2 = add_constant(vm->bytecode, w2);
    size_t index3 = add_constant(vm->bytecode, w3);

    add_opcode(vm->bytecode, OP_CONST, 0);
    add_operand(vm->bytecode, index1, 0);
    add_opcode(vm->bytecode, OP_CONST, 0);
    add_operand(vm->bytecode, index2, 0);
    add_opcode(vm->bytecode, OP_CONST, 0);
    add_operand(vm->bytecode, index3, 0);
    add_opcode(vm->bytecode, OP_ADD, 0);
    add_opcode(vm->bytecode, OP_ADD, 0);
    add_opcode(vm->bytecode, OP_HALT, 0);

    vm_run(vm);

    object_free(AS_OBJECT(w1));
    object_free(AS_OBJECT(w2));
    object_free(AS_OBJECT(w3));
    vm_free(vm);
    return 0;
}
#endif

#if LEXER_TEST
int main(void) {
    Ruja_Lexer* lexer = lexer_new("input.ruja");
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

#if PARSER_TEST
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
                        Ruja_Ast ast = ast_new_stmt(NULL, NULL);
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

#if AST_TEST
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

#if COMPILER_TEST
int main(void) {
    Ruja_Compiler* compiler = compiler_new();
    Ruja_Vm* vm = vm_new();

    if (compile(compiler, "input.ruja", vm) != RUJA_COMPILER_ERROR) {
        // disassemble(vm->bytecode, "code");
        vm_run(vm);
    }

    vm_free(vm);
    compiler_free(compiler);
    return 0;
}
#endif