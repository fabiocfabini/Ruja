#ifndef RUJA_OBJECT_STRING_H
#define RUJA_OBJECT_STRING_H

#include "common.h"
#include "objects.h"

// String object
typedef struct {
    Object obj;
    size_t length;
    char *chars;
} ObjString;

#define AS_STRING(x) ((ObjString*) AS_OBJECT(x))
#define MAKE_STRING(str, len) MAKE_OBJECT(object_new(OBJ_STRING, str, len))
#define IS_STRING(x) is_obj_type((x), OBJ_STRING)
ObjString* obj_string_new(const char* chars, size_t length);
ObjString* obj_string_new_no_alloc(char* chars, size_t length);
ObjString* string_add(ObjString* string1, ObjString* string2);
bool string_equal(ObjString* string1, ObjString* string2);

#endif // RUJA_OBJECT_STRING_H