#include "paging.h"

#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../lib/mem.h"
#include "../lib/panic.h"
#include "../lib/string.h"
#include "../mm/frame.h"

extern uint32_t free_memory_addr;

page_directory_t* current_directory = NULL;
page_directory_t* kernel_directory = NULL;

static void page_fault_handler(registers_t* r);
void alloc_page(page_t* page, int is_user_mode, int is_writeable);
void free_page(page_t* page);

void init_paging() {
	uint32_t mem_end_page = 0x1000000;	// 16MB
	init_frames(mem_end_page / 0x1000);

	kernel_directory =
		(page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
	memory_set((uint8_t*)kernel_directory, 0, sizeof(page_directory_t));
	current_directory = kernel_directory;

	uint32_t addr = 0;
	while (addr < free_memory_addr) {
		alloc_page(get_page(addr, 1, kernel_directory), 0, 0);
		addr += 0x1000;
	}
	register_interrupt_handler(14, page_fault_handler);
	switch_page_directory(kernel_directory);
}

void alloc_page(page_t* page, int is_user_mode, int is_writeable) {
	if (page->frame_addr != 0) {
		return;
	} else {
		uint32_t idx = alloc_frame();
		page->present = 1;
		page->writeable = (is_writeable) ? 1 : 0;
		page->usermode = (is_user_mode) ? 1 : 0;
		page->frame_addr = idx;
	}
}

void free_page(page_t* page) {
	uint32_t frame;
	if (!(frame = page->frame_addr)) {
		return;
	} else {
		clear_frame(frame);
		page->frame_addr = 0x0;
	}
}

void switch_page_directory(page_directory_t* dir) {
	current_directory = dir;
	asm volatile("mov %0, %%cr3" ::"r"(dir->physical_table));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

page_t* get_page(uint32_t address, int make, page_directory_t* dir) {
	address /= 0x1000;
	uint32_t table_idx = address / 1024;
	if (dir->virtual_table[table_idx]) {
		return &dir->virtual_table[table_idx]->pages[address % 1024];
	} else if (make) {
		uint32_t tmp;
		dir->virtual_table[table_idx] =
			(page_table_t*)kmalloc_aligned_with_phys(sizeof(page_table_t),
													 &tmp);
		memory_set((uint8_t*)dir->virtual_table[table_idx], 0, 0x1000);
		dir->physical_table[table_idx] = tmp | 0x7;	 // P, RW, US.
		return &dir->virtual_table[table_idx]->pages[address % 1024];
	} else {
		return 0;
	}
}

static void page_fault_handler(registers_t* r) {
	uint32_t faulting_address;
	__asm__ __volatile__("mov %%cr2, %0" : "=r"(faulting_address));

	int present = !(r->err_code & 0x1);	 // Page not present
	int writeable = r->err_code & 0x2;	 // Write operation?
	int usermode = r->err_code & 0x4;	 // Processor was in user-mode?
	int reserved = r->err_code & 0x8;	 // Overwritten CPU-reserved bits?

	kernel_print("Page fault! ( ");
	if (present) {
		kernel_print("present ");
	}
	if (writeable) {
		kernel_print("read-only ");
	}
	if (usermode) {
		kernel_print("user-mode ");
	}
	if (reserved) {
		kernel_print("reserved ");
	}
	kernel_print(") at 0x");
	char str[32];
	hex_to_ascii(faulting_address, str);
	kernel_print(str);
	kernel_print("\n");
	PANIC("PAGE FAULT!");
}
