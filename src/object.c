#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../includes/objects.h"

void print_object(FILE* stream, Object* obj) {
    switch (obj->type) {
        case OBJ_STRING:
            fprintf(stream, "%s", ((ObjString*)obj)->chars);
            break;
        default:
            fprintf(stream, "???");
            break;
    }
}

void object_free(Object* obj) {
    switch (obj->type) {
        case OBJ_STRING:
            free(((ObjString*)obj)->chars);
            break;
        default:
            break;
    }
    free(obj);
}

static ObjString* obj_string_new(const char* chars, size_t length) {
    ObjString* obj = malloc(sizeof(ObjString));
    if (obj == NULL) {
        fprintf(stderr, "Could not allocate memory for object\n");
        return NULL;
    }

    obj->obj.type = OBJ_STRING;
    obj->length = length;
    obj->chars = malloc(length + 1);
    if (obj->chars == NULL) {
        fprintf(stderr, "Could not allocate memory for object chars\n");
        free(obj);
        return NULL;
    }

    memcpy(obj->chars, chars, length);
    obj->chars[length] = '\0';

    return obj;
}

Object* object_new(object_type type, ...) {
    switch (type) {
        case OBJ_STRING: { // Expects a const char* and a size_t
            va_list args;
            va_start(args, type);

            const char* chars = va_arg(args, const char*);
            size_t length = va_arg(args, size_t);

            va_end(args);

            return (Object*)obj_string_new(chars, length);
        }
        default:
            fprintf(stderr, "Invalid object type: %d\n", type);
            return NULL;
    }
}