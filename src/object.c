#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../includes/objects.h"
#include "../includes/string.h"

void print_object(FILE* stream, Object* obj, int width) {
    switch (obj->type) {
        case OBJ_STRING:
            fprintf(stream, "%*s", width, ((ObjString*)obj)->chars);
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