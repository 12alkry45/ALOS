#ifndef LIB_PANIC_H
#define LIB_PANIC_H

#include "string.h"

extern void kernel_print(const char* message);
extern void int_to_ascii(int n, char str[]);

#define NORETURN __attribute__((noreturn))

NORETURN void panic(const char* msg);

#define PANIC(msg)                            \
	do {                                      \
		char line_buf[16];                    \
		atoi(__LINE__, line_buf, 10);         \
		kernel_print("KERNEL PANIC\n");       \
		kernel_print("FILE: " __FILE__ "\n"); \
		kernel_print("LINE: ");               \
		kernel_print(line_buf);               \
		kernel_print("\nMSG: " msg "\n");     \
		panic(msg);                           \
	} while (0);

#endif
