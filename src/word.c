#include <stdio.h>
#include <stdlib.h>

#include "../includes/word.h"
#include "../includes/memory.h"

Words* words_new() {
    Words* words = malloc(sizeof(Words));
    if (words == NULL) {
        fprintf(stderr, "Could not allocate memory for words\n");
        return NULL;
    }

    words->count = 0;
    words->capacity = 0;
    words->items = NULL;

    return words;
}

void words_free(Words* words) {
    free(words->items);
    free(words);
}