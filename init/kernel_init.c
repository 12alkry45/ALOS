#include <arch/isr.h>
#include <drivers/screen.h>
#include <lib/mem.h>
#include <lib/stdio.h>
#include <mm/paging.h>
#include <stdint.h>

void kernel_main() {
	isr_install();
	irq_install();
	clear_screen();
	uint32_t a = kmalloc(8);
	printf("a: %p\n", a);

	init_paging();

	uint32_t b = kmalloc(8);
	uint32_t c = kmalloc(8);
	printf("b: %p\n", b);
	printf("c: %p\n", c);

	kfree((uint32_t*)c);
	kfree((uint32_t*)b);
	uint32_t d = kmalloc(12);
	printf("d: %p\n", d);

	printf("Type something... END to halt the CPU\n> ");
}
