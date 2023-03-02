#include <stdio.h>
#include <stdlib.h>

#include "../includes/stack.h"
#include "../includes/memory.h"


Stack *stack_new() {
    Stack* stack = malloc(sizeof(Words));
    if (stack == NULL) {
        fprintf(stderr, "Could not allocate memory for stack\n");
        return NULL;
    }

    stack->count = 0;
    stack->capacity = 0;
    stack->items = NULL;

    return stack;
}

void stack_free(Stack *stack) {
    free(stack->items);
    free(stack);
}

void stack_push(Stack* stack, Word word) {
    if (stack->count >= stack->capacity) {
        REALLOC_DA(Word, stack);
    }

    stack->items[stack->count++] = word;
}

void stack_trace(Stack *stack) {
    printf("Stack: [ ");
    for (size_t i = 0; i < stack->count; i++) {
        printf("%4.2lf ", stack->items[i]);
    }
    printf(" ]\n");
}
