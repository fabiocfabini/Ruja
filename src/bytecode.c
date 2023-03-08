#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../includes/bytecode.h"
#include "../includes/memory.h"

Bytecode* bytecode_new() {
    Bytecode* bytecode = malloc(sizeof(Bytecode));
    if (bytecode == NULL) {
        fprintf(stderr, "Could not allocate memory for bytecode\n");
        return NULL;
    }

    bytecode->constant_words = words_new();
    if (bytecode->constant_words == NULL) return NULL;

    bytecode->count = 0;
    bytecode->capacity = 0;
    bytecode->items = NULL;
    bytecode->lines = NULL;

    return bytecode;
}

void bytecode_free(Bytecode* bytecode) {
    words_free(bytecode->constant_words);
    free(bytecode->items);
    free(bytecode->lines);
    free(bytecode);
}

size_t add_constant(Bytecode* bytecode, Word word) {
    if (bytecode->constant_words->count >= bytecode->constant_words->capacity) {
        REALLOC_DA(Word, bytecode->constant_words);
    }

    bytecode->constant_words->items[bytecode->constant_words->count++] = word;

    return bytecode->constant_words->count-1;
}

void add_opcode(Bytecode* bytecode, uint8_t byte, size_t line) {
    if (bytecode->count >= bytecode->capacity) {
        REALLOC_DA(uint8_t, bytecode);
        bytecode->lines = realloc(bytecode->lines, sizeof(size_t) * bytecode->capacity);
    }

    bytecode->items[bytecode->count] = byte;
    bytecode->lines[bytecode->count++] = line;
}

/**
 * @brief Disassembles a bytecode intruction into a human readable format
 * 
 * @param bytecode The bytecode to disassemble
 * @param index The index of the instruction to disassemble
 */
static void disassemble_instruction(Bytecode* bytecode, size_t* index) {
    Opcode opcode = bytecode->items[*index];
    switch (opcode) {
        default         : printf("%14s |%14s |", "Unknown", "-----"); break;
        case OP_HALT    : printf("%14s |%14s |", "HALT", "-----"); break;
        case OP_NIL     : printf("%14s |%14s |", "NIL", "-----"); break;
        case OP_TRUE    : printf("%14s |%14s |", "TRUE", "-----"); break;
        case OP_FALSE   : printf("%14s |%14s |", "FALSE", "-----"); break;
        case OP_NEG     : printf("%14s |%14s |", "NEG", "-----"); break;
        case OP_NOT     : printf("%14s |%14s |", "NOT", "-----"); break;
        case OP_ADD_F64 : printf("%14s |%14s |", "ADD_F64", "-----"); break;
        case OP_SUB_F64 : printf("%14s |%14s |", "SUB_F64", "-----"); break;
        case OP_MUL_F64 : printf("%14s |%14s |", "MUL_F64", "-----"); break;
        case OP_DIV_F64 : printf("%14s |%14s |", "DIV_F64", "-----"); break;
        case OP_EQ_F64  : printf("%14s |%14s |", "EQ_F64", "-----"); break;
        case OP_NEQ_F64 : printf("%14s |%14s |", "NEQ_F64", "-----"); break;
        case OP_LT_F64  : printf("%14s |%14s |", "LT_F64", "-----"); break;
        case OP_LTE_F64 : printf("%14s |%14s |", "LTE_F64", "-----"); break;
        case OP_GT_F64  : printf("%14s |%14s |", "GT_F64", "-----"); break;
        case OP_GTE_F64 : printf("%14s |%14s |", "GTE_F64", "-----"); break;
        case OP_AND     : printf("%14s |%14s |", "AND", "-----"); break;
        case OP_OR      : printf("%14s |%14s |", "OR", "-----"); break;
        case OP_JUMP    : {
            printf("%14s |", "JUMP");
            printf("%14d", bytecode->items[++(*index)]);
            printf(" |");
        } break;
        case OP_JZ      : {
            printf("%14s |", "JZ");
            printf("%14d", bytecode->items[++(*index)]);
            printf(" |");
        } break;
        case OP_CONST: {
            // printf("%14s |%14lf |", "CONST", bytecode->items[bytecode->items[++(*index)]]); break;
            printf("%14s |", "CONST");
            print_word(stdout, bytecode->constant_words->items[bytecode->items[++(*index)]], 14);
            printf(" |");
        } break;
    }
}

const char* opcode_to_string(Opcode opcode) {
    switch (opcode) {
        default         : return "Unknown";
        case OP_HALT    : return "HALT";
        case OP_NIL     : return "NIL";
        case OP_TRUE    : return "TRUE";
        case OP_FALSE   : return "FALSE";
        case OP_NEG     : return "NEG";
        case OP_NOT     : return "NOT";
        case OP_ADD_F64 : return "ADD_F64";
        case OP_SUB_F64 : return "SUB_F64";
        case OP_MUL_F64 : return "MUL_F64";
        case OP_DIV_F64 : return "DIV_F64";
        case OP_EQ_F64  : return "EQ_F64";
        case OP_NEQ_F64 : return "NEQ_F64";
        case OP_LTE_F64 : return "LTE_F64";
        case OP_LT_F64  : return "LT_F64";
        case OP_GT_F64  : return "GT_F64";
        case OP_GTE_F64 : return "GTE_F64";
        case OP_AND     : return "AND";
        case OP_OR      : return "OR";
        case OP_JUMP    : return "JUMP";
        case OP_JZ      : return "JZ";
        case OP_CONST   : return "CONST";
    }
}

void disassemble(Bytecode* bytecode, const char* name) {
    printf("---- %s ----\n", name);
    printf("%5s |%5s |%14s |%14s |\n", "IP", "Line", "Instruction", "Operand");
    for (size_t i = 0; i < bytecode->count; i++) {
        printf("%5"PRIu64" |", i);
        if (i == 0 || bytecode->lines[i] != bytecode->lines[i-1]) printf("%5"PRIu64" |", bytecode->lines[i]);
        else printf("    - |");
        disassemble_instruction(bytecode, &i);
        printf("\n");
    }
}

void save_bytecode(Bytecode* bytecode, const char* filename) {
    UNUSED(bytecode);
    UNUSED(filename);
    NOT_IMPLEMENTED("save_bytecode", __FILE__, __LINE__);
}

Bytecode* load_bytecode(const char* filename) {
    UNUSED(filename);
    NOT_IMPLEMENTED("load_bytecode", __FILE__, __LINE__);
    return NULL;
}
