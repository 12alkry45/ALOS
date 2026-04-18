#include "kernel.h"

#include "../cpu/isr.h"
#include "../drivers/screen.h"
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
	} else {
		kernel_print("You said: ");
		kernel_print(input);
		kernel_print("\n> ");
	}
}
