#ifndef LIB_STDIO_H
#define LIB_STDIO_H

#include <stdarg.h>
#include <stddef.h>

int printf(const char* fmt, ...);
int vsnprintf(const char* fmt, char* str, size_t str_size, va_list agrs);

#endif
