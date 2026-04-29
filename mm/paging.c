#include "paging.h"

#include <arch/isr.h>
#include <arch/memory.h>
#include <arch/register.h>
#include <drivers/screen.h>
#include <lib/mem.h>
#include <lib/panic.h>
#include <lib/stdio.h>
#include <lib/string.h>

#include "frame.h"
#include "mm/kheap.h"

extern heap_t* kheap;
extern uint32_t free_memory_addr;

page_directory_t* current_directory = NULL;
page_directory_t* kernel_directory = NULL;

static void page_fault_handler(registers_t* r);
void alloc_page(page_t* page, int is_user_mode, int is_writeable);
void free_page(page_t* page);

void init_paging() {
	init_frames();

	kernel_directory =
		(page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
	memset(kernel_directory, 0, sizeof(page_directory_t));
	current_directory = kernel_directory;

	for (unsigned int i = KHEAP_START; i < KHEAP_START + KHEAP_INITIALISE_SIZE;
		 i += 0x1000) {
		get_page(i, 1, kernel_directory);
	}

	uint32_t addr = 0;
	while (addr <= free_memory_addr) {
		alloc_page(get_page(addr, 1, kernel_directory), 0, 1);
		addr += 0x1000;
	}

	for (unsigned int i = KHEAP_START; i < KHEAP_START + KHEAP_INITIALISE_SIZE;
		 i += 0x1000) {
		alloc_page(get_page(i, 1, kernel_directory), 0, 1);
	}

	register_interrupt_handler(14, page_fault_handler);
	switch_page_directory(kernel_directory);
	kheap = create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIALISE_SIZE,
						0xCFFFF000, false, false);
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
	write_cr3((uint32_t)dir->physical_table);
	uint32_t cr0 = read_cr0();
	write_cr0(cr0 | 0x80000000);
}

page_t* get_page(uint32_t address, int make, page_directory_t* dir) {
	address /= 0x1000;
	uint32_t table_idx = address / 1024;
	if (dir->virtual_table[table_idx]) {
		return &dir->virtual_table[table_idx]->pages[address % 1024];
	} else if (make) {
		uint32_t phys;
		dir->virtual_table[table_idx] =
			(page_table_t*)kmalloc_aligned_with_phys(sizeof(page_table_t),
													 &phys);
		memset((void*)dir->virtual_table[table_idx], 0, 0x1000);
		dir->physical_table[table_idx] = phys | 0x7;  // P, RW, US.
		return &dir->virtual_table[table_idx]->pages[address % 1024];
	} else {
		return 0;
	}
}

static void page_fault_handler(registers_t* r) {
	uint32_t faulting_address = read_cr2();

	int present = !(r->err_code & 0x1);	 // Page not present
	int writeable = r->err_code & 0x2;	 // Write operation?
	int usermode = r->err_code & 0x4;	 // Processor was in user-mode?
	int reserved = r->err_code & 0x8;	 // Overwritten CPU-reserved bits?

	printf("Page fault! ( ");
	if (present) {
		printf("present ");
	}
	if (writeable) {
		printf("read-only ");
	}
	if (usermode) {
		printf("user-mode ");
	}
	if (reserved) {
		printf("reserved ");
	}
	printf(") at %p\n", faulting_address);
	PANIC("PAGE FAULT!");
}
