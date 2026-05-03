#ifndef LIB_PANIC_H
#define LIB_PANIC_H

#include "string.h"

#define NORETURN __attribute__((noreturn))

NORETURN void panic(const char* msg, const char* file, int line);

#define PANIC(msg) panic(msg, __FILE__, __LINE__)

#endif
