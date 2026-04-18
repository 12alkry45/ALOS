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
