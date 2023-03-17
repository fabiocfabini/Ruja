#ifndef RUJA_COMMON_H
#define RUJA_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#define RED "\033[1;31m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m" 

#define NOT_IMPLEMENTED(msg, file, line) \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented\n", file, line, msg); \
    } while (0)

#define UNUSED(x) (void)(x)
#define DEBUG_TOKENS 0

#endif // RUJA_COMMON_H