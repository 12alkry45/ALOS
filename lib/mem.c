#include "mem.h"

void memory_copy(uint8_t* source, uint8_t* dest, size_t nbytes) {
	for (size_t i = 0; i < nbytes; ++i) {
		*(dest + i) = *(source + i);
	}
}

void memory_set(uint8_t* dest, uint8_t val, uint32_t len) {
	uint8_t* temp = (uint8_t*)dest;
	for (; len != 0; len--) {
		*temp++ = val;
	}
}

uint32_t free_memory_addr = 0x100000;

uint32_t kernel_malloc(size_t size, int align, uint32_t* phys_addr) {
	if (align == 1 && (free_memory_addr & 0xFFF)) {
		free_memory_addr &= 0xFFFFF000;
		free_memory_addr += 0x1000;
	}
	if (phys_addr) *phys_addr = free_memory_addr;
	uint32_t return_addr = free_memory_addr;
	free_memory_addr += size;
	return return_addr;
}

uint32_t kmalloc(size_t size) { return kernel_malloc(size, 0, NULL); }

uint32_t kmalloc_aligned(size_t size) { return kernel_malloc(size, 1, NULL); }

uint32_t kmalloc_with_phys(size_t size, uint32_t* phys_addr) {
	return kernel_malloc(size, 0, phys_addr);
}

uint32_t kmalloc_aligned_with_phys(size_t size, uint32_t* phys_addr) {
	return kernel_malloc(size, 1, phys_addr);
}
