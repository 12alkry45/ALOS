#include "panic.h"

NORETURN void panic(const char* msg) {
	__asm__ __volatile__("cli");
	kernel_print(msg);
	while (1) __asm__ __volatile__("hlt");
}
