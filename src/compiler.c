#include <stdio.h>
#include <stdlib.h>

#include "../includes/compiler.h"
#include "../includes/parser.h"
#include "../includes/lexer.h"


Ruja_Compiler* compiler_new() {
    Ruja_Compiler* compiler = malloc(sizeof(Ruja_Compiler));
    if (compiler == NULL) {
        fprintf(stderr, "Could not allocate memory for compiler\n");
        return NULL;
    }

    compiler->ast = ast_new_expression(NULL);

    return compiler;
}

void compiler_free(Ruja_Compiler *compiler) {
    ast_free(compiler->ast);
    free(compiler);
}

static void push_word(Bytecode* bytecode, Word word) {
    switch (word & MASK_TYPE) {
        case TYPE_NIL: add_opcode(bytecode, OP_NIL, 0); break;
        case TYPE_BOOL: {
            if (AS_BOOL(word)) {
                add_opcode(bytecode, OP_TRUE, 0);
            } else {
                add_opcode(bytecode, OP_FALSE, 0);
            }
        } break;
        case TYPE_CHAR:
        case TYPE_INT:
        default: {
            size_t index = add_constant(bytecode, word);
            add_opcode(bytecode, OP_CONST, 0);
            add_operand(bytecode, index, 0);
        }
    }
}

static Ruja_Compile_Error compile_internal(Ruja_Ast ast, Bytecode* bytecode) {
    switch (ast->type) {
        case AST_NODE_EMPTY: {
            fprintf(stderr, "Empty AST\n");
            return RUJA_COMPILER_ERROR;
        }
        case AST_NODE_LITERAL: {
            push_word(bytecode, ast->as.literal.value);
        } break;
        case AST_NODE_UNARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.unary_op.expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            switch (ast->as.unary_op.type) {
                case AST_UNARY_OP_NOT: add_opcode(bytecode, OP_NOT, 0); break;
                case AST_UNARY_OP_NEG: add_opcode(bytecode, OP_NEG, 0); break;
            }
        } break;
        case AST_NODE_BINARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.binary_op.left_expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            error = compile_internal(ast->as.binary_op.right_expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            // TODO: Add typed binary operations to the parser. For now, we'll just assume that all literals are integers.
            switch (ast->as.binary_op.type) {
                case AST_BINARY_OP_ADD : add_opcode(bytecode, OP_ADD_F64, 0); break;
                case AST_BINARY_OP_SUB : add_opcode(bytecode, OP_SUB_F64, 0); break;
                case AST_BINARY_OP_MUL : add_opcode(bytecode, OP_MUL_F64, 0); break;
                case AST_BINARY_OP_DIV : add_opcode(bytecode, OP_DIV_F64, 0); break;
                case AST_BINARY_OP_EQ  : add_opcode(bytecode, OP_EQ_F64, 0); break;
                case AST_BINARY_OP_NE  : add_opcode(bytecode, OP_NEQ_F64, 0); break;
                case AST_BINARY_OP_LT  : add_opcode(bytecode, OP_LT_F64, 0); break;
                case AST_BINARY_OP_LE  : add_opcode(bytecode, OP_LTE_F64, 0); break;
                case AST_BINARY_OP_GT  : add_opcode(bytecode, OP_GT_F64, 0); break;
                case AST_BINARY_OP_GE  : add_opcode(bytecode, OP_GTE_F64, 0); break;
                case AST_BINARY_OP_AND : add_opcode(bytecode, OP_AND, 0); break;
                case AST_BINARY_OP_OR  : add_opcode(bytecode, OP_OR, 0); break;
            }
        } break;
        case AST_NODE_TERNARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.ternary_op.condition, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            add_opcode(bytecode, OP_JZ, 0);
            size_t jmp_false = bytecode->count;
            add_operand(bytecode, 0, 0);

            error = compile_internal(ast->as.ternary_op.true_expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            add_opcode(bytecode, OP_JUMP, 0);
            size_t jmp = bytecode->count;
            add_operand(bytecode, 0, 0);

            size_t operand_offset = jmp - jmp_false + 5;
            bytecode->items[jmp_false] = (uint8_t) ((operand_offset >> 24) & 0xFF);
            bytecode->items[jmp_false + 1] = (uint8_t) ((operand_offset >> 16) & 0xFF);
            bytecode->items[jmp_false + 2] = (uint8_t) ((operand_offset >> 8) & 0xFF);
            bytecode->items[jmp_false + 3] = (uint8_t) (operand_offset & 0xFF);

            error = compile_internal(ast->as.ternary_op.false_expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;

            operand_offset = bytecode->count - jmp + 1;
            bytecode->items[jmp] = (uint8_t) ((operand_offset >> 24) & 0xFF);
            bytecode->items[jmp + 1] = (uint8_t) ((operand_offset >> 16) & 0xFF);
            bytecode->items[jmp + 2] = (uint8_t) ((operand_offset >> 8) & 0xFF);
            bytecode->items[jmp + 3] = (uint8_t) (operand_offset & 0xFF);

        } break;
        case AST_NODE_EXPRESSION: {
            Ruja_Compile_Error error = compile_internal(ast->as.expr.expression, bytecode);
            if (error != RUJA_COMPILER_OK) return error;
        } break;
    }
    return RUJA_COMPILER_OK;
}

Ruja_Compile_Error compile(Ruja_Compiler *compiler, const char *source_path, Bytecode *bytecode) {
    Ruja_Lexer* lexer = NULL;
    Ruja_Parser* parser = NULL;

    lexer = lexer_new(source_path);
    if (lexer == NULL) goto error;

    parser = parser_new(lexer);
    if (parser == NULL) goto error;

    if (!parse(parser, lexer, &compiler->ast)) goto error;

    lexer_free(lexer); lexer = NULL;
    parser_free(parser); parser = NULL;
    // from this point we no longer have access to the source code. Let's see how it goes
    // if it's a problem we can always store the source code in the compiler struct

    if (compiler->ast->type != AST_NODE_EXPRESSION) {
        fprintf(stderr, "Only expressions are supported\n");
        goto error;
    }

    if (compile_internal(compiler->ast, bytecode)) {
        fprintf(stderr, "Could not compile\n");
        goto error;
    }

    add_opcode(bytecode, OP_HALT, 0);
    return RUJA_COMPILER_OK;

error:
    if (lexer != NULL) lexer_free(lexer);
    if (parser != NULL) parser_free(parser);
    return RUJA_COMPILER_ERROR;
}
