#include "isr.h"

#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "../lib/string.h"
#include "idt.h"
#include "ports.h"
#include "timer.h"

#define PORT_PIC_CTRL_MASTER 0x20
#define PORT_PIC_CTRL_SLAVE 0xA0
#define PORT_PIC_DATA_MASTER 0x21
#define PORT_PIC_DATA_SLAVE 0xA1

isr_t interupt_handlers[256];

static void set_up_PIC();

void isr_install() {
	set_idt_gate(0, (uint32_t)isr0);
	set_idt_gate(1, (uint32_t)isr1);
	set_idt_gate(2, (uint32_t)isr2);
	set_idt_gate(3, (uint32_t)isr3);
	set_idt_gate(4, (uint32_t)isr4);
	set_idt_gate(5, (uint32_t)isr5);
	set_idt_gate(6, (uint32_t)isr6);
	set_idt_gate(7, (uint32_t)isr7);
	set_idt_gate(8, (uint32_t)isr8);
	set_idt_gate(9, (uint32_t)isr9);
	set_idt_gate(10, (uint32_t)isr10);
	set_idt_gate(11, (uint32_t)isr11);
	set_idt_gate(12, (uint32_t)isr12);
	set_idt_gate(13, (uint32_t)isr13);
	set_idt_gate(14, (uint32_t)isr14);
	set_idt_gate(15, (uint32_t)isr15);
	set_idt_gate(16, (uint32_t)isr16);
	set_idt_gate(17, (uint32_t)isr17);
	set_idt_gate(18, (uint32_t)isr18);
	set_idt_gate(19, (uint32_t)isr19);
	set_idt_gate(20, (uint32_t)isr20);
	set_idt_gate(21, (uint32_t)isr21);
	set_idt_gate(22, (uint32_t)isr22);
	set_idt_gate(23, (uint32_t)isr23);
	set_idt_gate(24, (uint32_t)isr24);
	set_idt_gate(25, (uint32_t)isr25);
	set_idt_gate(26, (uint32_t)isr26);
	set_idt_gate(27, (uint32_t)isr27);
	set_idt_gate(28, (uint32_t)isr28);
	set_idt_gate(29, (uint32_t)isr29);
	set_idt_gate(30, (uint32_t)isr30);
	set_idt_gate(31, (uint32_t)isr31);

	set_up_PIC();

	set_idt_gate(32, (uint32_t)irq0);
	set_idt_gate(33, (uint32_t)irq1);
	set_idt_gate(34, (uint32_t)irq2);
	set_idt_gate(35, (uint32_t)irq3);
	set_idt_gate(36, (uint32_t)irq4);
	set_idt_gate(37, (uint32_t)irq5);
	set_idt_gate(38, (uint32_t)irq6);
	set_idt_gate(39, (uint32_t)irq7);
	set_idt_gate(40, (uint32_t)irq8);
	set_idt_gate(41, (uint32_t)irq9);
	set_idt_gate(42, (uint32_t)irq10);
	set_idt_gate(43, (uint32_t)irq11);
	set_idt_gate(44, (uint32_t)irq12);
	set_idt_gate(45, (uint32_t)irq13);
	set_idt_gate(46, (uint32_t)irq14);
	set_idt_gate(47, (uint32_t)irq15);

	set_idt();
}

static void set_up_PIC() {
	port_byte_out(PORT_PIC_CTRL_MASTER, 0x11);
	port_byte_out(PORT_PIC_CTRL_SLAVE, 0x11);
	// numbers
	port_byte_out(PORT_PIC_DATA_MASTER, 0x20);	// 32
	port_byte_out(PORT_PIC_DATA_SLAVE, 0x28);	// 40
	// master-slave
	port_byte_out(PORT_PIC_DATA_MASTER, 0x04);	// IRQ2, mask
	port_byte_out(PORT_PIC_DATA_SLAVE, 0x02);	// 2
	// mode 8086
	port_byte_out(PORT_PIC_DATA_MASTER, 0x01);
	port_byte_out(PORT_PIC_DATA_SLAVE, 0x01);
	// masks
	port_byte_out(PORT_PIC_DATA_MASTER, 0x0);
	port_byte_out(PORT_PIC_DATA_SLAVE, 0x0);
}

char* exception_messages[] = {"Division By Zero",
							  "Debug",
							  "Non Maskable Interrupt",
							  "Breakpoint",
							  "Into Detected Overflow",
							  "Out of Bounds",
							  "Invalid Opcode",
							  "No Coprocessor",

							  "Double Fault",
							  "Coprocessor Segment Overrun",
							  "Bad TSS",
							  "Segment Not Present",
							  "Stack Fault",
							  "General Protection Fault",
							  "Page Fault",
							  "Unknown Interrupt",

							  "Coprocessor Fault",
							  "Alignment Check",
							  "Machine Check",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",

							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved",
							  "Reserved"};

void isr_handler(registers_t* r) {
	if (interupt_handlers[r->int_num] != 0) {
		isr_t handler = interupt_handlers[r->int_num];
		handler(r);
	} else {
		kernel_print("\nINTERUPTION: ");
		char str[3];
		int_to_ascii(r->int_num, str);
		kernel_print(str);
		kernel_print("\n");
	}
}

void irq_handler(registers_t* r) {
	if (r->int_num >= 40) port_byte_out(PORT_PIC_CTRL_SLAVE, PIC_EOI);
	port_byte_out(PORT_PIC_CTRL_MASTER, PIC_EOI);

	if (interupt_handlers[r->int_num] != 0) {
		isr_t handler = interupt_handlers[r->int_num];
		handler(r);
	}
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
	interupt_handlers[n] = handler;
}

void irq_install() {
	__asm__ __volatile__("sti");
	init_timer(50);
	init_keyboard();
}
