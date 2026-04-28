#include <arch/isr.h>
#include <drivers/screen.h>
#include <lib/mem.h>
#include <lib/string.h>
#include <mm/paging.h>
#include <stdint.h>

void kernel_main() {
	isr_install();
	irq_install();
	clear_screen();
	uint32_t a = kmalloc(8);
	kernel_print("a: ");
	char str[16];
	atoi(a, str, 16);
	kernel_print(str);
	kernel_print("\n");

	init_paging();

	uint32_t b = kmalloc(8);
	uint32_t c = kmalloc(8);
	kernel_print("b: ");
	atoi(b, str, 16);
	kernel_print(str);
	kernel_print("\n");
	kernel_print("c: ");
	atoi(c, str, 16);
	kernel_print(str);
	kernel_print("\n");

	kfree((uint32_t*)c);
	kfree((uint32_t*)b);
	uint32_t d = kmalloc(12);
	kernel_print("d: ");
	atoi(d, str, 16);
	kernel_print(str);
	kernel_print("\n");

	kernel_print("Type something... END to halt the CPU\n> ");
}
