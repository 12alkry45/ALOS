#include <arch/gdt.h>
#include <arch/isr.h>
#include <drivers/screen.h>
#include <fs/ramfs.h>
#include <lib/mem.h>
#include <lib/stdio.h>
#include <mm/paging.h>
#include <stdint.h>
#include <test/test.h>

void kernel_main(unsigned long magic, unsigned long addr) {
	if (magic != 0x2BADB002) {
		return;
	}
	init_gdt();
	isr_install();
	irq_install();
	clear_screen();
	init_paging();
	clear_screen();
	// vfs_init();
	test_vfs_layer();

	printf("Type something... END to halt the CPU\n> ");
}
