#include "utils.h"

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

void int_to_ascii(int n, char str[]) {
	int i = 0, sign = n;
	if (sign < 0) n = -n;

	do {
		str[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0) str[i++] = '-';
	str[i] = '\0';

	reverse(str);
}

void reverse(char str[]) {
	char c = 0;
	for (int i = 0, j = strlen(str) - 1; i < j; i++, j--) {
		c = str[i];
		str[i] = str[j - i];
		str[j - i] = c;
	}
}

int strlen(char str[]) {
	int i = 0;
	while (str[i] != 0) ++i;
	return i;
}
