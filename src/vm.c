#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "../includes/vm.h"
#include "../includes/objects.h"
#include "../includes/string.h"
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
    vm->objects = NULL;
    vm->ip = NULL;

    return vm;
}

static void objects_free(Object* obj) {
    while (obj != NULL) {
        Object* next = obj->next;
        object_free(obj);
        obj = next;
    }
}

void vm_free(Ruja_Vm *vm) {
    bytecode_free(vm->bytecode);
    stack_free(vm->stack);
    objects_free(vm->objects);
    free(vm);
}

static void add_to_list(Ruja_Vm *vm, Object* obj) {
    obj->next = vm->objects;
    vm->objects = obj;
}

Object* vm_allocate_object(Ruja_Vm *vm, object_type type, ...) {
    switch (type) {
        case OBJ_STRING: {
            va_list args;
            va_start(args, type);

            const char* chars = va_arg(args, const char*);
            size_t length = va_arg(args, size_t);

            ObjString* obj = obj_string_new(chars, length);
            if (obj == NULL) return NULL;

            va_end(args);

            add_to_list(vm, (Object*) obj);

            return (Object*) obj;
        } break;
        default: {
            fprintf(stderr, "Unknown object type '%d'\n", type);
            return NULL;
        }
    }
}

Ruja_Vm_Status vm_run(Ruja_Vm *vm) {
    #define IP_NUMBER() ((size_t) (vm->ip - vm->bytecode->items))
    #define READ_BYTE(x) (*(vm->ip + (x)))

    #if 1
    disassemble(vm->bytecode, "VM RUN");
    #endif

    vm->ip = vm->bytecode->items;
    for (;;) {
        if (IP_NUMBER() >= vm->bytecode->count) {
            fprintf(stderr, "Ran out of bytecode\n");
            return RUJA_VM_ERROR;
        }

        #if 1

        printf("%10s ", opcode_to_string(READ_BYTE(0)));
        stack_trace(vm->stack);

        #endif

        Opcode opcode = *vm->ip++;
        switch (opcode) {
            default: {
                fprintf(stderr, "Unknown opcode at ip=%"PRIu64"\n", IP_NUMBER());
                return RUJA_VM_ERROR;
            }
            case OP_HALT: {
                return RUJA_VM_OK;
            }
            case OP_CONST: {
                size_t constant_index = (((size_t) READ_BYTE(0)) << 24) |
                                        (((size_t) READ_BYTE(1)) << 16) |
                                        (((size_t) READ_BYTE(2)) << 8) |
                                        (((size_t) READ_BYTE(3)));
                stack_push(vm->stack, vm->bytecode->constants->items[constant_index]);
                vm->ip += 4;
            } break;
            case OP_NIL: {
                stack_push(vm->stack, MAKE_NIL());
                
            } break;
            case OP_TRUE: {
                stack_push(vm->stack, MAKE_BOOL(true));
                
            } break;
            case OP_FALSE: {
                stack_push(vm->stack, MAKE_BOOL(false));
                
            } break;
            case OP_NEG: {
                if (vm->stack->count < 1) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                if (IS_DOUBLE(word)) {
                    vm->stack->items[vm->stack->count-1] = MAKE_DOUBLE(-(AS_DOUBLE(word)));
                } else if (IS_INT(word)) {
                    vm->stack->items[vm->stack->count-1] = MAKE_INT(-(AS_INT(word)));
                } else {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid type for negation in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                
            } break;
            case OP_NOT: {
                if (vm->stack->count < 1) {
                    fprintf(stderr, RED"ERROR: "WHITE"Stack underflow at ip=%"PRIu64"\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                if (IS_OBJECT(word)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid type for negation in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                vm->stack->items[vm->stack->count-1] = MAKE_BOOL(!AS_BOOL(word));

                
            } break;
            case OP_ADD : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (IS_DOUBLE(word1) && IS_DOUBLE(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) + AS_DOUBLE(word2));
                    vm->stack->count--;
                    
                } else if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else if (IS_INT(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) + AS_INT(word2));
                    vm->stack->count--;
                    
                } else if (IS_STRING(word1)) {
                    ObjString *string3 = string_add(AS_STRING(word1), AS_STRING(word2));
                    if (string3 == NULL) {
                        fprintf(stderr, RED"ERROR: "WHITE"Out of memory while concatenating strings in ip '%zu' VM.\n"RESET, IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                    add_to_list(vm, (Object*) string3);

                    vm->stack->items[vm->stack->count-2] = MAKE_OBJECT(string3);
                    vm->stack->count--;
                    
                } else {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }
            } break;
            case OP_SUB : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (IS_DOUBLE(word1) && IS_DOUBLE(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) - AS_DOUBLE(word2));
                    vm->stack->count--;
                    
                } else if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else if (IS_INT(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) - AS_INT(word2));
                    vm->stack->count--;
                    
                } else {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }
            } break;
            case OP_MUL : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (IS_DOUBLE(word1) && IS_DOUBLE(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) * AS_DOUBLE(word2));
                    vm->stack->count--;
                    
                } else if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else if (IS_INT(word1)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) * AS_INT(word2));
                    vm->stack->count--;
                    
                } else {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }
            } break;
            case OP_DIV : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (IS_DOUBLE(word1) && IS_DOUBLE(word1)) {
                    if (AS_DOUBLE(word2) == 0.0) {
                        fprintf(stderr, "Division by zero at ip=%"PRIu64"\n", IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                    vm->stack->items[vm->stack->count-2] = MAKE_DOUBLE(AS_DOUBLE(word1) * AS_DOUBLE(word2));
                    vm->stack->count--;
                    
                } else if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else if (IS_INT(word1)) {
                    if (AS_INT(word2) == 0) {
                        fprintf(stderr, "Division by zero at ip=%"PRIu64"\n", IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                    vm->stack->items[vm->stack->count-2] = MAKE_INT(AS_INT(word1) * AS_INT(word2));
                    vm->stack->count--;
                    
                } else {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for addition in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                }
            } break;
            case OP_EQ  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_BOOL(false);
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) == AS_DOUBLE(word2));
                    } else if (IS_STRING(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(string_equal(AS_STRING(word1), AS_STRING(word2)));
                    } else {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(word1 == word2);
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_NEQ : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    vm->stack->items[vm->stack->count-2] = MAKE_BOOL(true);
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) != AS_DOUBLE(word2));
                    } else if (IS_STRING(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(!string_equal(AS_STRING(word1), AS_STRING(word2)));
                    } else {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(word1 != word2);
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_LT  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for '<' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) < AS_DOUBLE(word2));
                    } else if (IS_INT(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) < AS_INT(word2));
                    } else {
                        fprintf(stderr, RED"BUG: "WHITE"Invalid types for '<' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_LTE : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for '<=' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) <= AS_DOUBLE(word2));
                    } else if (IS_INT(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) <= AS_INT(word2));
                    } else {
                        fprintf(stderr, RED"BUG: "WHITE"Invalid types for '<=' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_GT  : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for '>' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) > AS_DOUBLE(word2));
                    } else if (IS_INT(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) > AS_INT(word2));
                    } else {
                        fprintf(stderr, RED"BUG: "WHITE"Invalid types for '>' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_GTE : {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                if (TYPE(word1) != TYPE(word2)) {
                    fprintf(stderr, RED"BUG: "WHITE"Invalid types for '>=' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                    return RUJA_VM_ERROR;
                } else {
                    if (IS_DOUBLE(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_DOUBLE(word1) >= AS_DOUBLE(word2));
                    } else if (IS_INT(word1)) {
                        vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_INT(word1) >= AS_INT(word2));
                    } else {
                        fprintf(stderr, RED"BUG: "WHITE"Invalid types for '>=' in ip '%zu' VM. This is probably a bug in the type checking.\n"RESET, IP_NUMBER());
                        return RUJA_VM_ERROR;
                    }
                }

                vm->stack->count--;
                
            } break;
            case OP_AND: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) && AS_BOOL(word2));

                vm->stack->count--;
                
            } break;
            case OP_OR: {
                if (vm->stack->count < 2) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word1 = vm->stack->items[vm->stack->count-2];
                Word word2 = vm->stack->items[vm->stack->count-1];

                vm->stack->items[vm->stack->count-2] = MAKE_BOOL(AS_BOOL(word1) || AS_BOOL(word2));

                vm->stack->count--;
                
            } break;
            case OP_JUMP: {
                size_t operand = (((size_t) READ_BYTE(0)) << 24) |
                                (((size_t)  READ_BYTE(1)) << 16) |
                                (((size_t)  READ_BYTE(2)) << 8) |
                                (((size_t)  READ_BYTE(3)));
                vm->ip += operand - 1;
            } break;
            case OP_JZ: {
                if (vm->stack->count < 1) {
                    fprintf(stderr, "Stack underflow at ip=%"PRIu64"\n", IP_NUMBER());
                    return RUJA_VM_ERROR;
                }

                Word word = vm->stack->items[vm->stack->count-1];
                vm->stack->count--;

                if (AS_BOOL(word)) {
                    vm->ip += 4;
                } else {
                    size_t operand = (((size_t) READ_BYTE(0)) << 24) |
                                    (((size_t)  READ_BYTE(1)) << 16) |
                                    (((size_t)  READ_BYTE(2)) << 8) |
                                    (((size_t)  READ_BYTE(3)));
                    vm->ip += operand - 1;
                }
            } break;
        }
    }
}
