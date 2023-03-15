#ifndef RUJA_OBJECTS_H
#define RUJA_OBJECTS_H

#include "common.h"
#include "word.h"

typedef enum {
    OBJ_STRING,
} object_type;

// Base object type
typedef struct Object {
    object_type type;
    struct Object* next;
} Object;

#define AS_OBJECT(x) ((Object*) ((x) & MASK_VALUE))
static inline bool is_obj_type(Word value, object_type type) {
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

void object_free(Object* obj);
void print_object(FILE* stream, Object* obj, int width);

#endif // RUJA_OBJECTS_H