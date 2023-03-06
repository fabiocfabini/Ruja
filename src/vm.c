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
            case OP_NEG: {
                Word word = vm->stack->items[vm->stack->count-1];
                if (!IS_DOUBLE(word) && !IS_INT(word)) {
                    fprintf(stderr, "Cannot negate non-numerical value at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word)) {
                    vm->stack->items[vm->stack->count-1] = MAKE_INT(-AS_INT(word));
                } else {
                    vm->stack->items[vm->stack->count-1] = MAKE_DOUBLE(-(AS_DOUBLE(word)));
                }

                vm->ip++;
            } break;
            case OP_NOT: {
                if (!IS_BOOL(vm->stack->items[vm->stack->count-1])) {
                    fprintf(stderr, "Cannot negate non-bool value at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                vm->stack->items[vm->stack->count-1] = MAKE_BOOL(!AS_BOOL(word));

                vm->ip++;
            } break;
            case OP_ADD: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];
                if ((!IS_INT(word1) && !IS_DOUBLE(word1)) || (!IS_INT(word2) && !IS_DOUBLE(word2))) {
                    fprintf(stderr, "Cannot add non-numerical values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word1) && IS_INT(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) + AS_INT(word2));
                } else {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) + AS_DOUBLE(word2));
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_SUB: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];
                if ((!IS_INT(word1) && !IS_DOUBLE(word1)) || (!IS_INT(word2) && !IS_DOUBLE(word2))) {
                    fprintf(stderr, "Cannot subtract non-numerical values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word1) && IS_INT(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) - AS_INT(word2));
                } else {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) - AS_DOUBLE(word2));
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_MUL: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];
                if ((!IS_INT(word1) && !IS_DOUBLE(word1)) || (!IS_INT(word2) && !IS_DOUBLE(word2))) {
                    fprintf(stderr, "Cannot multiply non-numerical values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word1) && IS_INT(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) * AS_INT(word2));
                } else {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) * AS_DOUBLE(word2));
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_DIV: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];
                if ((!IS_INT(word1) && !IS_DOUBLE(word1)) || (!IS_INT(word2) && !IS_DOUBLE(word2))) {
                    fprintf(stderr, "Cannot divide non-numerical values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word2) && AS_INT(word2) == 0) {
                    fprintf(stderr, "Cannot divide by zero at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                if (IS_INT(word1) && IS_INT(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) / AS_INT(word2));
                } else {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) / AS_DOUBLE(word2));
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_EQ: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) == AS_BOOL(word2));
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) == AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) == AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) == AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_NEQ: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) != AS_BOOL(word2));
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) != AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) != AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) != AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LT: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        fprintf(stderr, "Cannot compare boolean values with '<' operator at ip=%"PRIu64"\n", vm->ip);
                        return RUJA_VM_ERROR;
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) < AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) < AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) < AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_LTE: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        fprintf(stderr, "Cannot compare boolean values with '<=' operator at ip=%"PRIu64"\n", vm->ip);
                        return RUJA_VM_ERROR;
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) <= AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) <= AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) <= AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GT: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        fprintf(stderr, "Cannot compare boolean values with '>' operator at ip=%"PRIu64"\n", vm->ip);
                        return RUJA_VM_ERROR;
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) > AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) > AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) > AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

                vm->stack->count--;
                vm->ip++;
            } break;
            case OP_GTE: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if ((word1 & MASK_TYPE) != (word2 & MASK_TYPE)) {
                    fprintf(stderr, "Cannot compare values of different types at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                switch (word1 & MASK_TYPE) {
                    case TYPE_NAN: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_NIL: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                    } break;
                    case TYPE_BOOL: {
                        fprintf(stderr, "Cannot compare boolean values with '>=' operator at ip=%"PRIu64"\n", vm->ip);
                        return RUJA_VM_ERROR;
                    } break;
                    case TYPE_CHAR: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_CHAR(word1) >= AS_CHAR(word2));
                    } break;
                    case TYPE_INT: {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) >= AS_INT(word2));
                    } break;
                    default: {
                        if (IS_DOUBLE(word1)) {
                            vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) >= AS_DOUBLE(word2));
                            break;
                        } else {
                            fprintf(stderr, "Cannot compare values of different type at ip=%"PRIu64"\n", vm->ip);
                            return RUJA_VM_ERROR;
                        }
                    }
                }

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
                if (!IS_BOOL(word2) && !IS_BOOL(word1)) {
                    fprintf(stderr, "Cannot perform logical AND on non-boolean values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

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
                if (!IS_BOOL(word2) && !IS_BOOL(word1)) {
                    fprintf(stderr, "Cannot perform logical OR on non-boolean values at ip=%"PRIu64"\n", vm->ip);
                    return RUJA_VM_ERROR;
                }

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) || AS_BOOL(word2));

                vm->stack->count--;
                vm->ip++;
            } break;
        }
    }
}
