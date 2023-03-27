#ifndef RUJA_TYPES_H
#define RUJA_TYPES_H

#include "common.h"

typedef enum {
    VAR_TYPE_NIL,
    VAR_TYPE_BOOL,
    VAR_TYPE_CHAR,
    VAR_TYPE_I32,
    VAR_TYPE_F64,
    VAR_TYPE_STRING,
} Var_Type; //NOTE: better name?

#endif // RUJA_TYPES_H