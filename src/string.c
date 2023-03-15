#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/string.h"

ObjString* obj_string_new(const char* chars, size_t length) {
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

ObjString* obj_string_new_no_alloc(char* chars, size_t length) {
    ObjString* obj = malloc(sizeof(ObjString));
    if (obj == NULL) {
        fprintf(stderr, "Could not allocate memory for object\n");
        return NULL;
    }

    obj->obj.type = OBJ_STRING;
    obj->length = length;
    obj->chars = chars;

    return obj;
}

ObjString* string_add(ObjString* string1, ObjString* string2) {
    size_t length = string1->length + string2->length;
    char* chars = malloc(length + 1);
    if (chars == NULL) {
        fprintf(stderr, "Could not allocate memory for string chars\n");
        return NULL;
    }

    memcpy(chars, string1->chars, string1->length);
    memcpy(chars + string1->length, string2->chars, string2->length);
    chars[length] = '\0';

    return obj_string_new_no_alloc(chars, length);
}

bool string_equal(ObjString* string1, ObjString* string2) {
    if (string1->length != string2->length) {
        return false;
    }

    return memcmp(string1->chars, string2->chars, string1->length) == 0;
}