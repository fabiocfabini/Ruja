#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "includes/parser.h"
#include "includes/bytecode.h"

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
    printf("  -v, --version\t\tPrint the version of Ruja.\n");
}

int main() {
    Bytecode* bytecode = bytecode_new();

    add_opcode(bytecode, OP_CONST, 1);
    size_t index1 = add_constant(bytecode, 1.2);
    add_opcode(bytecode, (uint8_t)(index1), 1);
    add_opcode(bytecode, OP_CONST, 1);
    size_t index2 = add_constant(bytecode, 1.2);
    add_opcode(bytecode, (uint8_t)(index2), 1);
    add_opcode(bytecode, OP_ADD, 1);
    add_opcode(bytecode, OP_SUB, 1);
    add_opcode(bytecode, OP_MUL, 1);
    add_opcode(bytecode, OP_DIV, 1);
    add_opcode(bytecode, OP_HALT, 2);

    disassemble(bytecode, "test");

    bytecode_free(bytecode);
    return 0;
}

int main2(int argc, char** argv) {
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
                Ruja_Lexer* lexer = lexer_new(*argv);
                if (lexer != NULL) {
                    Ruja_Parser* parser = parser_new();
                    if (parser != NULL) {
                        parse(parser, lexer);

                        parser_free(parser);
                    }
                    lexer_free(lexer);
                }
            } else {
                printf("Unknown option '%s'.\n", *argv);
                usage(); return 1;
            }
        }
    }

    return 0;
}