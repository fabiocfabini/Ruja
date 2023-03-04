#ifndef RUJA_COMMON_H
#define RUJA_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#define RED     "\x1B[31m"
#define RESET   "\x1B[0m"

#define NOT_IMPLEMENTED(msg, file, line) \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented\n", file, line, msg); \
    } while (0)

#endif // RUJA_COMMON_H