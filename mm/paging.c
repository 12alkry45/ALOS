#include "paging.h"

#include "../cpu/isr.h"
#include "../lib/mem.h"
#include "../lib/panic.h"
#include "frame.h"

static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

extern uint32_t free_memory_addr;

static void page_fault_handler(registers_t* r);

void init_paging() {
	kernel_directory =
		(page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
	memory_set((uint8_t*)kernel_directory, 0, sizeof(page_directory_t));
	current_directory = kernel_directory;
	uint32_t addr = 0;
	while (addr < free_memory_addr) {
		paging_map(addr, addr, false, true);  // kernel and writeable
		addr += 0x1000;
	}
	register_interrupt_handler(14, page_fault_handler);
	switch_page_directory(kernel_directory);
}

page_t* get_page(uint32_t virt_addr, int create_table, page_directory_t* dir) {
	virt_addr >>= 12;
	uint32_t table_idx = virt_addr / 1024;
	if (dir->virtual_table[table_idx]) {
		return &dir->virtual_table[table_idx]->pages[virt_addr % 1024];
	} else if (create_table) {
		uintptr_t phys;
		dir->virtual_table[table_idx] =
			(page_table_t*)kmalloc_aligned_with_phys(sizeof(page_table_t),
													 &phys);
		memory_set((uint8_t*)dir->virtual_table[table_idx], 0, 0x1000);
		dir->physical_table[table_idx] = phys | 0x7;  // P, RW, USER
		return &dir->virtual_table[table_idx]->pages[virt_addr % 1024];
	}
	return NULL;
}

void paging_map(uintptr_t virt_addr, uintptr_t phys_addr, bool user,
				bool writeable) {
	page_t* page = get_page(virt_addr, true, current_directory);
	if (phys_addr == 0) phys_addr = alloc_frame();
	page->present = 1;
	page->writeable = writeable ? 1 : 0;
	page->usermode = user ? 1 : 0;
	page->frame_addr = phys_addr >> 12;
}

void paging_unmap(uintptr_t virt_addr) {
	page_t* page = get_page(virt_addr, false, current_directory);
	if (page && page->present) {
		free_frame(page->frame_addr << 12);
		page->present = 0;
		page->frame_addr = 0;
	}
}

void switch_page_directory(page_directory_t* dir) {
	current_directory = dir;
	__asm__ __volatile__("mov %0, %%cr3" : : "r"(&dir->physical_table));
	uint32_t cr0;
	__asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	__asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0));
}

static void page_fault_handler(registers_t* r) { PANIC("PAGE FAULT!"); }
