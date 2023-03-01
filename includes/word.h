#ifndef RUJA_WORD_H
#define RUJA_WORD_H

#include "common.h"

typedef double Word;
typedef struct {
    size_t count;
    size_t capacity;
    Word* items;
} Words;

Words* words_new();
void words_free(Words* words);

#endif // RUJA_WORD_H