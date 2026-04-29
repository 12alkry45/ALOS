#include "panic.h"

#include "stdio.h"

NORETURN void panic(const char* msg, const char* file, int line) {
	__asm__ __volatile__("cli");
	printf("KERNEL PANIC\n");
	printf("FILE: %s\n", file);
	printf("LINE: %d\n", line);
	printf("MSG: %s\n", msg);
	while (1) __asm__ __volatile__("hlt");
}
