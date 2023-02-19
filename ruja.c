#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "src/lexer.h"

char* read_file(const char* filepath){
    char* buffer = NULL;
    
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Could not open file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    if (fseek(file, 0L, SEEK_END) == -1) {
        printf("Could not seek to end of file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        printf("Could not get size of file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    if (fseek(file, 0L, SEEK_SET) == -1) {
        printf("Could not seek to start of file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Could not allocate memory for file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    size_t new_len = fread(buffer, sizeof(char), (size_t) file_size, file);
    if (ferror(file) != 0) {
        printf("Could not read file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }
    buffer[new_len++] = '\0';

    if (fclose(file) == EOF) {
        printf("Could not close file '%s': %s\n", filepath, strerror(errno));
        if (buffer != NULL) {
            free(buffer);
        }
        return NULL;
    }

    return buffer;
}

void shift_agrs(int* argc, char*** argv) {
    (*argc)--;
    (*argv)++;
}

bool endswith(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) {
        return false;
    }
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void usage() {
    printf("Usage: <path-to>/ruja <input-file> [OPTIONS]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help\t\tPrint this help message.\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(); return 1;
    } else {
        while (argc > 1) {
            shift_agrs(&argc, &argv);
            if (strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
                usage(); return 0;
            } else if (strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0) {
                printf("Ruja 0.0.1\n"); return 0;
            } else if (endswith(*argv, ".ruja")) {
                char* source = read_file(*argv);
                if (source != NULL){
                    Ruja_Lexer lexer = lexer_new(source);

                    while (true) {
                        Ruja_Token token = next_token(&lexer);
                        token_to_string(&token);
                        if (token.kind == RUJA_TOK_EOF) break;
                    }

                    free(source);
                }
            } else {
                printf("Unknown option '%s'.\n", *argv);
                usage(); return 1;
            }
        }
    }

    return 0;
}