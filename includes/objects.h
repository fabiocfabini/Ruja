#ifndef RUJA_OBJECTS_H
#define RUJA_OBJECTS_H

#include "common.h"

typedef enum {
    OBJ_STRING,
} object_type;

// Base object type
typedef struct {
    object_type type;
} Object;

// String object
typedef struct {
    Object obj;
    size_t length;
    char *chars;
} ObjString;


Object* object_new(object_type type, ...);
void object_free(Object* obj);

void print_object(FILE* stream, Object* obj, int width);

#endif // RUJA_OBJECTS_H