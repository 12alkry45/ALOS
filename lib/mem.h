#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

uint32_t kernel_malloc(size_t size, int align, uint32_t* phys_addr);
uint32_t kmalloc(size_t size);
uint32_t kmalloc_aligned(size_t size);
uint32_t kmalloc_with_phys(size_t size, uint32_t* phys_addr);
uint32_t kmalloc_aligned_with_phys(size_t size, uint32_t* phys_addr);
void kfree(void* p);

#endif
