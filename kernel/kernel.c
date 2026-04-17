#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "utils.h"

void main() {
	isr_install();

	clear_screen();
	__asm__ __volatile__("int $6");
	__asm__ __volatile__("int $15");
}
