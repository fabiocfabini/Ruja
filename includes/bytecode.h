#ifndef RUJA_BYTECODE_H
#define RUJA_BYTECODE_H

#include "common.h"
#include "word.h"

typedef enum {
    OP_HALT,
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
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