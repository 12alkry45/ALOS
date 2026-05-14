#include <arch/isr.h>
#include <drivers/screen.h>
#include <fs/ramfs.h>
#include <lib/mem.h>
#include <lib/stdio.h>
#include <mm/paging.h>
#include <stdint.h>
#include <test/test.h>

void kernel_main() {
	isr_install();
	irq_install();
	clear_screen();
	init_paging();
	clear_screen();
	test_ramfs();
	test_ramfs_creation();

	printf("Type something... END to halt the CPU\n> ");
}
