#ifndef RUJA_OBJECTS_H
#define RUJA_OBJECTS_H

#include "common.h"
#include "word.h"

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

#define AS_OBJECT(x) ((Object*) ((x) & MASK_VALUE))
static inline bool is_obj_type(Word value, object_type type) {
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

void object_free(Object* obj);

void print_object(FILE* stream, Object* obj, int width);


#define AS_STRING(x) ((ObjString*) AS_OBJECT(x))
#define MAKE_STRING(str, len) MAKE_OBJECT(object_new(OBJ_STRING, str, len))
#define IS_STRING(x) is_obj_type((x), OBJ_STRING)
ObjString* obj_string_new(const char* chars, size_t length);
ObjString* string_concatenate(ObjString* string1, ObjString* string2);

#endif // RUJA_OBJECTS_H