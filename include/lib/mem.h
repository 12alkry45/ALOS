#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void* kernel_malloc(size_t size, int align, uint32_t* phys_addr);
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size);
void* kmalloc_with_phys(size_t size, uint32_t* phys_addr);
void* kmalloc_aligned_with_phys(size_t size, uint32_t* phys_addr);
void kfree(void* p);

#endif
