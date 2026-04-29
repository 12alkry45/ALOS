#include "kernel.h"

#include <drivers/screen.h>
#include <lib/mem.h>
#include <lib/stdio.h>
#include <lib/string.h>

void user_input(char* input) {
	if (strcmp(input, "END") == 0) {
		printf("Stopping the CPU... Bye!\n");
		__asm__ __volatile__("hlt");
	} else if (strcmp(input, "LS") == 0) {
		printf("No file system here:)\n> ");
	} else if (strcmp(input, "PAGE") == 0) {
		uint32_t* phys_addr;
		uint32_t* page = (uint32_t*)kmalloc_aligned_with_phys(1000, phys_addr);
		printf("Page: %p, physical addr: %p\n> ", page, phys_addr);
	} else {
		printf("You said: %s\n> ", input);
	}
}
