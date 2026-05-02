#ifndef ARCH_MEMORY_H
#define ARCH_MEMORY_H

#include <stddef.h>
#include <stdint.h>

static inline void* memset(void* dest, int c, size_t n) {
	void* t_dest = dest;
	size_t t_n = n;
	__asm__ __volatile__(
		"cld\n\t"
		"rep stosb"
		: "+D"(t_dest), "+c"(t_n)
		: "a"((unsigned char)c)
		: "memory");
	return dest;
}

static inline void* memcpy(void* dest, const void* src, size_t n) {
	void* t_dest = dest;
	const void* t_src = src;
	size_t t_n = n;
	__asm__ __volatile__(
		"cld\n\t"
		"rep movsb"
		: "+D"(t_dest), "+S"(t_src), "+c"(t_n)
		:
		: "memory");
	return dest;
}

#endif
