#include "timer.h"

#include "../drivers/ports.h"
#include "../drivers/screen.h"
#include "isr.h"

uint32_t tick = 0;

static void timer_callback(registers_t r) {
	tick++;
	kernel_print("Tick: ");
	char tick_ascii[256];
	int_to_ascii(tick, tick_ascii);
	kernel_print(tick_ascii);
	kernel_print("\n");
}

void init_timer(uint32_t frequency) {
	register_interrupt_handler(IRQ0, timer_callback);
    
	uint32_t divisor = 1193180 / frequency;
	uint8_t low = (uint8_t)(divisor & 0xFF);
	uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

	port_byte_out(PORT_TIMER_CTRL, 0x36);
	port_byte_out(PORT_TIMER_DATA, low);
	port_byte_out(PORT_TIMER_DATA, high);
}
