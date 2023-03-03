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
    #if 0
    disassemble(vm->bytecode, "VM RUN");
    #endif

    for (;;) {
        if (vm->ip >= vm->bytecode->count) {
            fprintf(stderr, "Ran out of bytecode\n");
            return RUJA_VM_ERROR;
        }

        #if 1

        printf("ip=%5"PRIu64" [%8s]", vm->ip, opcode_to_string(vm->bytecode->items[vm->ip]));
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
                Word word = vm->bytecode->constant_words->items[vm->bytecode->items[vm->ip+1]];
                stack_push(vm->stack, word);
                vm->ip += 2;
            } break;
            case OP_NEG: {
                vm->stack->items[vm->stack->count-1] = -vm->stack->items[vm->stack->count-1];
                vm->ip++;
            } break;
            case OP_NOT: {
                vm->stack->items[vm->stack->count-1] = !vm->stack->items[vm->stack->count-1];
                vm->ip++;
            } break;
            case OP_ADD: {
                vm->stack->items[vm->stack->count-2] += vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_SUB: {
                vm->stack->items[vm->stack->count-2] -= vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_MUL: {
                vm->stack->items[vm->stack->count-2] *= vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_DIV: {
                if (vm->stack->items[vm->stack->count-1] == 0) {
                    fprintf(stderr, "Division by zero at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                vm->stack->items[vm->stack->count-2] /= vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_EQ: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] == vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_NEQ: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] != vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LT: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] < vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LTE: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] <= vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GT: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] > vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GTE: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] >= vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_AND: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] && vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_OR: {
                vm->stack->items[vm->stack->count-2] = vm->stack->items[vm->stack->count-2] || vm->stack->items[vm->stack->count-1];
                vm->stack->count--;
                vm->ip++;
            } break;
        }
    }
}
