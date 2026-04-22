#include "panic.h"

extern void kernel_print(const char* message);

NORETURN void panic(const char* msg) {
	__asm__ __volatile__("cli");
	kernel_print(msg);
	while (1) __asm__ __volatile__("hlt");
}
