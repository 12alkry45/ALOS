#include "utils.h"

void memory_copy(char* source, char* dest, int nbytes) {
	for (int i = 0; i < nbytes; ++i) {
		*(dest + i) = *(source + i);
	}
}

void int_to_ascii(int n, char str[]) {
	int i = 0, sign = n;
	if (sign < 0) n = -n;

	do {
		str[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0) str[i++] = '-';

    for(int j = 0; j < i / 2; j++) {
        char c;
        c = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = c;
    }
	str[i] = '\0';
}
