#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "utils.h"

void main() {
	isr_install();

	clear_screen();
	__asm__ __volatile__("sti");
	// init_timer(19);
	init_keyboard();
}
