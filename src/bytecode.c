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

static void disassemble_instruction(Bytecode* bytecode, size_t* index) {
    static_assert(OP_COUNT == 7, "OP_COUNT must be 7");
    switch (bytecode->items[*index]) {
        default:       printf("%14s |%14s |", "Unknown", "-----"); break;
        case OP_HALT:  printf("%14s |%14s |", "HALT", "-----"); break;
        case OP_ADD:   printf("%14s |%14s |", "ADD", "-----"); break;
        case OP_NEG:   printf("%14s |%14s |", "NEG", "-----"); break;
        case OP_SUB:   printf("%14s |%14s |", "SUB", "-----"); break;
        case OP_MUL:   printf("%14s |%14s |", "MUL", "-----"); break;
        case OP_DIV:   printf("%14s |%14s |", "DIV", "-----"); break;
        case OP_CONST: printf("%14s |%14lf |", "CONST", bytecode->constant_words->items[bytecode->items[++(*index)]]); break;
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

}

Bytecode* load_bytecode(const char* filename) {
    return NULL;
}
