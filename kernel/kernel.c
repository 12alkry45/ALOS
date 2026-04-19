#include "kernel.h"

#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../lib/mem.h"
#include "../lib/string.h"

void main() {
	isr_install();
	irq_install();
	clear_screen();

	kernel_print("Type something... END to halt the CPU\n> ");
}

void user_input(char* input) {
	if (strcmp(input, "END") == 0) {
		kernel_print("Stopping the CPU... Bye!\n");
		__asm__ __volatile__("hlt");
	} else if (strcmp(input, "LS") == 0) {
		kernel_print("No file system here:)\n> ");
	} else if (strcmp(input, "PAGE") == 0) {
		uint32_t phys_addr;
		uint32_t page = kernel_malloc(1000, 1, &phys_addr);
		char page_str[16] = "";
		hex_to_ascii(page, page_str);
		char phys_str[16] = "";
		hex_to_ascii(phys_addr, phys_str);
		kernel_print("Page: ");
		kernel_print(page_str);
		kernel_print(", physical address: ");
		kernel_print(phys_str);
		kernel_print("\n> ");
	} else {
		kernel_print("You said: ");
		kernel_print(input);
		kernel_print("\n> ");
	}
}
