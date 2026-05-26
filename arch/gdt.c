#include <arch/gdt.h>
#include <stdint.h>

void init_gdt() {
	extern uint16_t gdt_descriptor;

	__asm__ volatile(
		"lgdt (%0)\n"
		"movw %1, %%ax\n"
		"movw %%ax, %%ds\n"
		"movw %%ax, %%es\n"
		"movw %%ax, %%fs\n"
		"movw %%ax, %%gs\n"
		"movw %%ax, %%ss\n"

		"pushl %2\n"
		"pushl $1f\n"
		"retf\n"
		"1:\n"
		:
		: "r"(&gdt_descriptor), "i"(DATA_SEG), "i"(CODE_SEG)
		: "ax", "memory");
}
