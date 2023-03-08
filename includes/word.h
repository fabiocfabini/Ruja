#ifndef RUJA_WORD_H
#define RUJA_WORD_H

#include <stdio.h>

#include "common.h"

typedef uint64_t Word;

// Types
#define TYPE_NAN   0x7FF8000000000000
#define TYPE_NIL   0x7FF9000000000000
#define TYPE_BOOL  0x7FFA000000000000
#define TYPE_CHAR  0x7FFB000000000000
#define TYPE_INT   0x7FFC000000000000

// Mask
#define MASK_TYPE  0x7FFF000000000000
#define MASK_VALUE 0x0000FFFFFFFFFFFF
#define TYPE(x)    ((x) & MASK_TYPE)
#define VALUE(x)   ((x) & MASK_VALUE)

// Checks
#define IS_NAN(x)   (((x) & MASK_TYPE) == TYPE_NAN)
#define IS_NIL(x)   (((x) & MASK_TYPE) == TYPE_NIL)
#define IS_BOOL(x)  (((x) & MASK_TYPE) == TYPE_BOOL)
#define IS_CHAR(x)  (((x) & MASK_TYPE) == TYPE_CHAR)
#define IS_INT(x)   (((x) & MASK_TYPE) == TYPE_INT)
#define IS_DOUBLE(x) (((x) & TYPE_NAN) != TYPE_NAN)

// Makes
#define MAKE_NAN()     ((TYPE_NAN))
#define MAKE_NIL()     ((TYPE_NIL))
#define MAKE_BOOL(x)   ((TYPE_BOOL | (x)))
#define MAKE_CHAR(x)   ((TYPE_CHAR | (x)))
#define MAKE_INT(x)    ((TYPE_INT  | (uint32_t) (x)))
#define MAKE_DOUBLE(x) double_to_word(x)

// Gets
#define AS_CHAR(x)  ((char) ((x) & MASK_VALUE))
#define AS_BOOL(x)  as_bool(x)
#define AS_INT(x)   ((int32_t) ((x) & MASK_VALUE)) 
#define AS_DOUBLE(x) word_to_double(x)

static inline Word double_to_word(double num) {
    union {
        uint64_t bits;
        double num;
    } data;
    data.num = num;
    return data.bits;
}

static inline double word_to_double(Word value) {
    union {
        uint64_t bits;
        double num;
    } data;
    data.bits = value;
    return data.num;
}

static inline Word as_bool(Word value) {
    if(IS_DOUBLE(value)) {
        return AS_DOUBLE(value) != 0.0;
    }
    return (bool) ((value) & MASK_VALUE);
}

void print_word(FILE* stream, Word w, int width);

typedef struct {
    size_t count;
    size_t capacity;
    Word* items;
} Words;

Words* words_new();
void words_free(Words* words);

#endif // RUJA_WORD_H