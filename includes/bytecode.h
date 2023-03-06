#ifndef RUJA_BYTECODE_H
#define RUJA_BYTECODE_H

#include "common.h"
#include "word.h"

typedef enum {
    OP_HALT,

    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    OP_NOT,
    OP_NEG,

    OP_ADD_F64,
    OP_ADD_I32,
    OP_SUB_F64,
    OP_SUB_I32,
    OP_MUL_F64,
    OP_MUL_I32,
    OP_DIV_F64,
    OP_DIV_I32,

    OP_EQ_F64,
    OP_EQ_I32,
    OP_NEQ_F64,
    OP_NEQ_I32,
    OP_LT_F64,
    OP_LT_I32,
    OP_LTE_F64,
    OP_LTE_I32,
    OP_GT_F64,
    OP_GT_I32,
    OP_GTE_F64,
    OP_GTE_I32,

    OP_AND,
    OP_OR,

    OP_CONST,
} Opcode;

typedef struct {
    size_t count;
    size_t capacity;
    uint8_t* items;
    size_t*  lines;

    Words* constant_words;

} Bytecode;

Bytecode* bytecode_new();
void bytecode_free(Bytecode* bytecode);

size_t add_constant(Bytecode* bytecode, Word word);
void add_opcode(Bytecode* bytecode, uint8_t byte, size_t line);

const char* opcode_to_string(Opcode opcode);
void disassemble(Bytecode* bytecode, const char* name);

void save_bytecode(Bytecode* bytecode, const char* filename);
Bytecode* load_bytecode(const char* filename);



#endif // RUJA_BYTECODE_H