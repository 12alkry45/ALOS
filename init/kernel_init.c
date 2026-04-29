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
	void* a = kmalloc(8);
	printf("a: %p\n", a);

	init_paging();

	void* b = kmalloc(8);
	void* c = kmalloc(8);
	printf("b: %p\n", b);
	printf("c: %p\n", c);

	kfree(c);
	kfree(b);
	void* d = kmalloc(12);
	printf("d: %p\n", d);

	printf("Type something... END to halt the CPU\n> ");
}
