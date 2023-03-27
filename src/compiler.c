#include <stdio.h>
#include <stdlib.h>

#include "../includes/compiler.h"
#include "../includes/objects.h"
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

static void push_word(Ruja_Vm* vm, Ruja_Token* token) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (token->kind) {
        case RUJA_TOK_NIL: add_opcode(vm->bytecode, OP_NIL, token->line); break;
        case RUJA_TOK_FALSE: add_opcode(vm->bytecode, OP_FALSE, token->line); break;
        case RUJA_TOK_TRUE: add_opcode(vm->bytecode, OP_TRUE, token->line); break;
        case RUJA_TOK_INT: {
            Word word = MAKE_INT(strtod(token->start, NULL));
            size_t index = add_constant(vm->bytecode, word);
            add_opcode(vm->bytecode, OP_CONST, token->line);
            add_operand(vm->bytecode, index, token->line);
        } break;
        case RUJA_TOK_FLOAT: {
            Word word = MAKE_DOUBLE(strtod(token->start, NULL));
            size_t index = add_constant(vm->bytecode, word);
            add_opcode(vm->bytecode, OP_CONST, token->line);
            add_operand(vm->bytecode, index, token->line);
        } break;
        case RUJA_TOK_CHAR: {
            Word word = MAKE_CHAR(*(token->start));
            size_t index = add_constant(vm->bytecode, word);
            add_opcode(vm->bytecode, OP_CONST, token->line);
            add_operand(vm->bytecode, index, token->line);
        } break;
        case RUJA_TOK_STRING: {
            Word word = MAKE_OBJECT(vm_allocate_object(vm, OBJ_STRING, token->start, token->length));
            size_t index = add_constant(vm->bytecode, word);
            add_opcode(vm->bytecode, OP_CONST, token->line);
            add_operand(vm->bytecode, index, token->line);
        } break;
        default: {
            fprintf(stderr, "Unknown token kind: %d (%s)\n", token->kind, token->start);
        }
    }
#pragma GCC diagnostic pop
}

static Ruja_Compile_Error compile_internal(Ruja_Ast ast, Ruja_Vm* vm) {
    Bytecode* bytecode = vm->bytecode;
    switch (ast->type) {
        case AST_NODE_EMPTY: {
            fprintf(stderr, "Empty AST\n");
            return RUJA_COMPILER_ERROR;
        }
        case AST_NODE_LITERAL: {
            push_word(vm, ast->as.literal.tok_literal);
        } break;
        case AST_NODE_UNARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.unary_op.expression, vm);
            if (error != RUJA_COMPILER_OK) return error;
            Ruja_Token* tok_unary = ast->as.unary_op.tok_unary;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
            switch (tok_unary->kind) {
                case RUJA_TOK_NOT: add_opcode(bytecode, OP_NOT, tok_unary->line); break;
                case RUJA_TOK_SUB: add_opcode(bytecode, OP_NEG, tok_unary->line); break;
            }
#pragma GCC diagnostic pop
        } break;
        case AST_NODE_BINARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.binary_op.left_expression, vm);
            if (error != RUJA_COMPILER_OK) return error;

            error = compile_internal(ast->as.binary_op.right_expression, vm);
            if (error != RUJA_COMPILER_OK) return error;

            Ruja_Token* tok_binary = ast->as.binary_op.tok_binary;

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wswitch"
            switch (tok_binary->kind) {
                case RUJA_TOK_ADD : add_opcode(bytecode, OP_ADD, tok_binary->line); break;
                case RUJA_TOK_SUB : add_opcode(bytecode, OP_SUB, tok_binary->line); break;
                case RUJA_TOK_MUL : add_opcode(bytecode, OP_MUL, tok_binary->line); break;
                case RUJA_TOK_DIV : add_opcode(bytecode, OP_DIV, tok_binary->line); break;
                case RUJA_TOK_EQ  : add_opcode(bytecode, OP_EQ, tok_binary->line); break;
                case RUJA_TOK_NE  : add_opcode(bytecode, OP_NEQ, tok_binary->line); break;
                case RUJA_TOK_LT  : add_opcode(bytecode, OP_LT, tok_binary->line); break;
                case RUJA_TOK_LE  : add_opcode(bytecode, OP_LTE, tok_binary->line); break;
                case RUJA_TOK_GT  : add_opcode(bytecode, OP_GT, tok_binary->line); break;
                case RUJA_TOK_GE  : add_opcode(bytecode, OP_GTE, tok_binary->line); break;
                case RUJA_TOK_AND : add_opcode(bytecode, OP_AND, tok_binary->line); break;
                case RUJA_TOK_OR  : add_opcode(bytecode, OP_OR, tok_binary->line); break;
            }
            #pragma GCC diagnostic pop
        } break;
        case AST_NODE_TERNARY_OP: {
            Ruja_Compile_Error error = compile_internal(ast->as.ternary_op.condition, vm);
            if (error != RUJA_COMPILER_OK) return error;

            add_opcode(bytecode, OP_JZ, ast->as.ternary_op.tok_ternary.tok_question->line);
            size_t jmp_false = bytecode->count;
            add_operand(bytecode, 0, ast->as.ternary_op.tok_ternary.tok_question->line);

            error = compile_internal(ast->as.ternary_op.true_expression, vm);
            if (error != RUJA_COMPILER_OK) return error;

            add_opcode(bytecode, OP_JUMP, ast->as.ternary_op.tok_ternary.tok_colon->line);
            size_t jmp = bytecode->count;
            add_operand(bytecode, 0, ast->as.ternary_op.tok_ternary.tok_colon->line);

            size_t operand_offset = jmp - jmp_false + 5;
            bytecode->items[jmp_false] = (uint8_t) ((operand_offset >> 24) & 0xFF);
            bytecode->items[jmp_false + 1] = (uint8_t) ((operand_offset >> 16) & 0xFF);
            bytecode->items[jmp_false + 2] = (uint8_t) ((operand_offset >> 8) & 0xFF);
            bytecode->items[jmp_false + 3] = (uint8_t) (operand_offset & 0xFF);

            error = compile_internal(ast->as.ternary_op.false_expression, vm);
            if (error != RUJA_COMPILER_OK) return error;

            operand_offset = bytecode->count - jmp + 1;
            bytecode->items[jmp] = (uint8_t) ((operand_offset >> 24) & 0xFF);
            bytecode->items[jmp + 1] = (uint8_t) ((operand_offset >> 16) & 0xFF);
            bytecode->items[jmp + 2] = (uint8_t) ((operand_offset >> 8) & 0xFF);
            bytecode->items[jmp + 3] = (uint8_t) (operand_offset & 0xFF);

        } break;
        case AST_NODE_EXPRESSION: {
            Ruja_Compile_Error error = compile_internal(ast->as.expr.expression, vm);
            if (error != RUJA_COMPILER_OK) return error;
        } break;
    }
    return RUJA_COMPILER_OK;
}

Ruja_Compile_Error compile(Ruja_Compiler *compiler, const char *source_path, Ruja_Vm* vm) {
    Ruja_Lexer* lexer = NULL;
    Ruja_Parser* parser = NULL;
    Ruja_Ir* ir = NULL;

    lexer = lexer_new(source_path);
    if (lexer == NULL) goto error;

    parser = parser_new();
    if (parser == NULL) goto error;

    ir = ir_new();
    if (ir == NULL) goto error;

    if (!parse(parser, lexer, &ir->ast, ir->symbol_table)) goto error;

    if (compiler->ast->type != AST_NODE_EXPRESSION) {
        fprintf(stderr, "Only expressions are supported\n");
        goto error;
    }

    if (compile_internal(compiler->ast, vm)) {
        fprintf(stderr, "Could not compile\n");
        goto error;
    }

    lexer_free(lexer); lexer = NULL;
    parser_free(parser); parser = NULL;
    // from this point we no longer have access to the source code. Let's see how it goes
    // if it's a problem we can always store the source code in the compiler struct
    add_opcode(vm->bytecode, OP_HALT, 0);
    return RUJA_COMPILER_OK;

error:
    if (lexer != NULL) lexer_free(lexer);
    if (parser != NULL) parser_free(parser);
    if (ir != NULL) ir_free(ir);
    return RUJA_COMPILER_ERROR;
}
