#include "mem.h"

#include <mm/kheap.h>
#include <mm/paging.h>

extern heap_t* kheap;
extern page_directory_t* kernel_directory;

uint32_t free_memory_addr = 0x100000;

uint32_t kernel_malloc(size_t size, int align, uint32_t* phys_addr) {
	if (kheap != 0) {
		void* addr = alloc(size, align != 0, kheap);
		if (phys_addr != 0) {
			page_t* page = get_page((uint32_t)addr, 0, kernel_directory);
			*phys_addr =
				page->frame_addr * PAGE_SIZE + ((uint32_t)addr & 0xFFF);
		}
		return (uint32_t)addr;
	} else {
		if (align == 1 && (free_memory_addr & 0xFFF)) {
			free_memory_addr &= 0xFFFFF000;
			free_memory_addr += 0x1000;
		}
		if (phys_addr) *phys_addr = free_memory_addr;
		uint32_t return_addr = free_memory_addr;
		free_memory_addr += size;
		return return_addr;
	}
}

void kfree(void* p) { free(p, kheap); }

uint32_t kmalloc(size_t size) { return kernel_malloc(size, 0, NULL); }

uint32_t kmalloc_aligned(size_t size) { return kernel_malloc(size, 1, NULL); }

uint32_t kmalloc_with_phys(size_t size, uint32_t* phys_addr) {
	return kernel_malloc(size, 0, phys_addr);
}

uint32_t kmalloc_aligned_with_phys(size_t size, uint32_t* phys_addr) {
	return kernel_malloc(size, 1, phys_addr);
}
