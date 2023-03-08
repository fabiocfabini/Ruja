#include <stdio.h>
#include <stdlib.h>

#include "../includes/vm.h"
#include "../includes/memory.h"


Ruja_Vm *vm_new() {
    Bytecode *bytecode = bytecode_new();
    if (bytecode == NULL) return NULL;

    Stack *stack = stack_new();
    if (stack == NULL) {
        bytecode_free(bytecode);
        return NULL;
    }

    Ruja_Vm *vm = malloc(sizeof(Ruja_Vm));
    if (vm == NULL) {
        bytecode_free(bytecode);
        stack_free(stack);
        return NULL;
    }

    vm->bytecode = bytecode;
    vm->stack = stack;
    vm->ip = 0;

    return vm;
}

void vm_free(Ruja_Vm *vm) {
    bytecode_free(vm->bytecode);
    stack_free(vm->stack);
    free(vm);
}

Ruja_Vm_Status vm_run(Ruja_Vm *vm) {
    #if 1
    disassemble(vm->bytecode, "VM RUN");
    #endif

    for (;;) {
        if (vm->ip >= vm->bytecode->count) {
            fprintf(stderr, "Ran out of bytecode\n");
            return RUJA_VM_ERROR;
        }

        #if 1

        stack_trace(vm->stack);

        #endif

        Opcode opcode = vm->bytecode->items[vm->ip];
        switch (opcode) {
            default: {
                fprintf(stderr, "Unknown opcode at ip=%"PRIu64"\n", vm->ip);
                return RUJA_VM_ERROR;
            }
            case OP_HALT: {
                return RUJA_VM_OK;
            }
            case OP_CONST: {
                stack_push(vm->stack, vm->bytecode->constant_words->items[vm->bytecode->items[vm->ip+1]]);
                vm->ip += 2;
            } break;
            case OP_NIL: {
                stack_push(vm->stack, MAKE_NIL());
                vm->ip++;
            } break;
            case OP_TRUE: {
                stack_push(vm->stack, MAKE_BOOL(true));
                vm->ip++;
            } break;
            case OP_FALSE: {
                stack_push(vm->stack, MAKE_BOOL(false));
                vm->ip++;
            } break;
            case OP_NEG: {
                if (vm->stack->count < 1) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                vm->stack->items[vm->stack->count-1] = MAKE_DOUBLE(-(AS_DOUBLE(word)));

                vm->ip++;
            } break;
            case OP_NOT: {
                if (vm->stack->count < 1) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                vm->stack->items[vm->stack->count-1] = MAKE_BOOL(!AS_BOOL(word));

                vm->ip++;
            } break;
            case OP_ADD_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) + AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_SUB_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) - AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_MUL_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) * AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_DIV_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) / AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_EQ_F64  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) == AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_NEQ_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) != AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LT_F64  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) < AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LTE_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) <= AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GT_F64  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) > AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GTE_F64 : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) >= AS_DOUBLE(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_AND: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) && AS_BOOL(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_OR: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) || AS_BOOL(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
        }
    }

#undef BINARY_OP
#undef COMPARISON_OP
}
