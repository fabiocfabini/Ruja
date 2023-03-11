#include <stdio.h>
#include <stdlib.h>

#include "../includes/word.h"
#include "../includes/objects.h"
#include "../includes/memory.h"

void print_word(FILE* stream, Word w, int width) {
    switch (w & MASK_TYPE) {
        case TYPE_NAN:
            fprintf(stream, "NAN");
            break;
        case TYPE_NIL:
            fprintf(stream, "NIL");
            break;
        case TYPE_BOOL:
            fprintf(stream, "%*s", width, AS_BOOL(w) ? "true" : "false");
            break;
        case TYPE_INT:
            fprintf(stream, "%*" PRId32 "", width, AS_INT(w));
            break;
        case TYPE_CHAR:
            fprintf(stream, "%*c", width, AS_CHAR(w));
            break;
        case TYPE_OBJ:
            print_object(stream, AS_OBJECT(w), width);
            break;
        default:
            if (IS_DOUBLE(w))
                fprintf(stream, "%*lf", width, AS_DOUBLE(w));
            else
                fprintf(stream, "???");
            break;
    }
}