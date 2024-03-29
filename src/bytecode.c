#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../includes/bytecode.h"
#include "../includes/memory.h"

Constants* constants_new() {
    Constants* contants = malloc(sizeof(Constants));
    if (contants == NULL) {
        fprintf(stderr, "Could not allocate memory for contants\n");
        return NULL;
    }

    contants->count = 0;
    contants->capacity = 0;
    contants->items = NULL;

    return contants;
}

void constants_free(Constants* constants) {
    free(constants->items);
    free(constants);
}

Bytecode* bytecode_new() {
    Bytecode* bytecode = malloc(sizeof(Bytecode));
    if (bytecode == NULL) {
        fprintf(stderr, "Could not allocate memory for bytecode\n");
        return NULL;
    }

    bytecode->constants = constants_new();
    if (bytecode->constants == NULL) return NULL;

    bytecode->count = 0;
    bytecode->capacity = 0;
    bytecode->items = NULL;
    bytecode->lines = NULL;

    return bytecode;
}

void bytecode_free(Bytecode* bytecode) {
    constants_free(bytecode->constants);
    free(bytecode->items);
    free(bytecode->lines);
    free(bytecode);
}

size_t add_constant(Bytecode* bytecode, Word word) {
    if (bytecode->constants->count >= bytecode->constants->capacity) {
        REALLOC_DA(Word, bytecode->constants);
    }

    bytecode->constants->items[bytecode->constants->count++] = word;

    return bytecode->constants->count-1;
}

void add_opcode(Bytecode* bytecode, uint8_t byte, size_t line) {
    if (bytecode->count >= bytecode->capacity) {
        REALLOC_DA(uint8_t, bytecode);
        bytecode->lines = realloc(bytecode->lines, sizeof(size_t) * bytecode->capacity);
    }

    bytecode->items[bytecode->count] = byte;
    bytecode->lines[bytecode->count++] = line;
}

void add_operand(Bytecode* bytecode, size_t bytes, size_t line) {
    if (bytecode->count + 4 >= bytecode->capacity) {
        REALLOC_DA(uint8_t, bytecode);
        bytecode->lines = realloc(bytecode->lines, sizeof(size_t) * bytecode->capacity);
    }

    bytecode->items[bytecode->count] = (bytes >> 24) & 0xFF;
    bytecode->items[bytecode->count+1] = (bytes >> 16) & 0xFF;
    bytecode->items[bytecode->count+2] = (bytes >> 8) & 0xFF;
    bytecode->items[bytecode->count+3] = bytes & 0xFF;
    bytecode->lines[bytecode->count] = line;
    bytecode->lines[bytecode->count+1] = line;
    bytecode->lines[bytecode->count+2] = line;
    bytecode->lines[bytecode->count+3] = line;
    bytecode->count += 4;
}

void print_operand(Bytecode* bytecode, size_t index, int format) {
    size_t operand = (bytecode->items[index] << 24) | (bytecode->items[index+1] << 16) | (bytecode->items[index+2] << 8) | bytecode->items[index+3];
    printf("%*ld", format, operand);
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
        default         : printf("%14s |%20s |", "Unknown", "-----"); break;
        case OP_HALT    : printf("%14s |%20s |", "HALT", "-----"); break;
        case OP_NIL     : printf("%14s |%20s |", "NIL", "-----"); break;
        case OP_TRUE    : printf("%14s |%20s |", "TRUE", "-----"); break;
        case OP_FALSE   : printf("%14s |%20s |", "FALSE", "-----"); break;
        case OP_NEG     : printf("%14s |%20s |", "NEG", "-----"); break;
        case OP_NOT     : printf("%14s |%20s |", "NOT", "-----"); break;
        case OP_ADD     : printf("%14s |%20s |", "ADD", "-----"); break;
        case OP_SUB     : printf("%14s |%20s |", "SUB", "-----"); break;
        case OP_MUL     : printf("%14s |%20s |", "MUL", "-----"); break;
        case OP_DIV     : printf("%14s |%20s |", "DIV", "-----"); break;
        case OP_EQ      : printf("%14s |%20s |", "EQ", "-----"); break;
        case OP_NEQ     : printf("%14s |%20s |", "NEQ", "-----"); break;
        case OP_LT      : printf("%14s |%20s |", "LT", "-----"); break;
        case OP_LTE     : printf("%14s |%20s |", "LTE", "-----"); break;
        case OP_GT      : printf("%14s |%20s |", "GT", "-----"); break;
        case OP_GTE     : printf("%14s |%20s |", "GTE", "-----"); break;
        case OP_AND     : printf("%14s |%20s |", "AND", "-----"); break;
        case OP_OR      : printf("%14s |%20s |", "OR", "-----"); break;
        case OP_JUMP    : {
            printf("%14s |", "JUMP");
            print_operand(bytecode, ++(*index), 20); *index += 3;
            printf(" |");
        } break;
        case OP_JZ      : {
            printf("%14s |", "JZ");
            print_operand(bytecode, ++(*index), 20); *index += 3;
            printf(" |");
        } break;
        case OP_CONST: {
            // printf("%20s |%20lf |", "CONST", bytecode->items[bytecode->items[++(*index)]]); break;
            printf("%14s |", "CONST");
            size_t constant_index = (bytecode->items[*index+1] << 24) |
                                    (bytecode->items[*index+2] << 16) | 
                                    (bytecode->items[*index+3] <<  8) | 
                                    bytecode->items[*index+4];
            print_word(stdout, bytecode->constants->items[constant_index], 20); *index += 4;
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
        case OP_ADD     : return "ADD";
        case OP_SUB     : return "SUB";
        case OP_MUL     : return "MUL";
        case OP_DIV     : return "DIV";
        case OP_EQ      : return "EQ";
        case OP_NEQ     : return "NEQ";
        case OP_LTE     : return "LTE";
        case OP_LT      : return "LT";
        case OP_GT      : return "GT";
        case OP_GTE     : return "GTE";
        case OP_AND     : return "AND";
        case OP_OR      : return "OR";
        case OP_JUMP    : return "JUMP";
        case OP_JZ      : return "JZ";
        case OP_CONST   : return "CONST";
    }
}

void disassemble(Bytecode* bytecode, const char* name) {
    printf("---- %s ----\n", name);
    printf("%5s |%5s |%14s |%20s |\n", "IP", "Line", "Instruction", "Operand");
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
