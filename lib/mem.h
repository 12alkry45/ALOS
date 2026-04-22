#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void memory_copy(uint8_t* source, uint8_t* dest, size_t nbytes);
void memory_set(uint8_t* dest, uint8_t val, uint32_t len);
uint32_t kernel_malloc(size_t size, int align, uint32_t* phys_addr);
uint32_t kmalloc_aligned(size_t size);
uint32_t kmalloc_with_phys(size_t size, uint32_t* phys_addr);
uint32_t kmalloc_aligned_with_phys(size_t size, uint32_t* phys_addr);

#endif
