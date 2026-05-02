#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>

static inline void write_cr3(uint32_t cr3) {
	__asm__ __volatile__("mov %0, %%cr3" ::"r"(cr3));
}

static inline uint32_t read_cr3() {
	uint32_t value;
	__asm__ __volatile__("mov %%cr3, %0" : "=r"(value));
	return value;
}

static inline void write_cr0(uint32_t cr0) {
	__asm__ __volatile__("mov %0, %%cr0" ::"r"(cr0));
}

static inline uint32_t read_cr0() {
	uint32_t value;
	__asm__ __volatile__("mov %%cr0, %0" : "=r"(value));
	return value;
}

static inline void write_cr2(uint32_t cr2) {
	__asm__ __volatile__("mov %0, %%cr0" ::"r"(cr2));
}

static inline uint32_t read_cr2() {
	uint32_t value;
	__asm__ __volatile__("mov %%cr2, %0" : "=r"(value));
	return value;
}

#endif
