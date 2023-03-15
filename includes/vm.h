#ifndef RUJA_VM_H
#define RUJA_VM_H

#include "common.h"
#include "bytecode.h"
#include "stack.h"
#include "objects.h"

typedef enum {
    RUJA_VM_ERROR = -1,
    RUJA_VM_OK,
} Ruja_Vm_Status;

typedef struct {
    Bytecode *bytecode;
    Stack* stack;
    Object* objects;

    uint8_t* ip;
} Ruja_Vm;

Ruja_Vm *vm_new();
void vm_free(Ruja_Vm *vm);

Object* vm_allocate_object(Ruja_Vm *vm, object_type type, ...);

Ruja_Vm_Status vm_run(Ruja_Vm *vm);


#endif // RUJA_VM_H