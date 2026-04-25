#include <arch/isr.h>
#include <drivers/screen.h>
#include <mm/paging.h>
#include <stdint.h>

void kernel_main() {
	isr_install();
	irq_install();
	clear_screen();
	init_paging();

	uint32_t* ptr = (uint32_t*)0xA0000000;
	uint32_t page_fault = *ptr;
	kernel_print("Type something... END to halt the CPU\n> ");
}
