#ifndef RUJA_STACK_H
#define RUJA_STACK_H

#include "common.h"
#include "word.h"

typedef Words Stack;

Stack *stack_new();
void stack_free(Stack *stack);

void stack_push(Stack *stack, Word word);

void stack_trace(Stack *stack);

#endif // RUJA_STACK_H