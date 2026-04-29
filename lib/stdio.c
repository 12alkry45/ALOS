#include "stdio.h"

#include <drivers/screen.h>

#include "lib/string.h"

int printf(const char* fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(fmt, buf, sizeof(buf), args);
	va_end(args);
	kernel_print(buf);
	return strlen(buf);
}

int vsnprintf(const char* fmt, char* buf, size_t buf_size, va_list args) {
	size_t i = 0;
	char num_buf[64];
	for (const char* ptr = fmt; *ptr != '\0'; ptr++) {
		if (i >= buf_size - 1) break;
		if (*ptr != '%') {
			buf[i++] = *ptr;
			continue;
		}
		ptr++;
		switch (*ptr) {
			case 's': {
				const char* str = va_arg(args, const char*);
				while (*str != '\0' && i < buf_size - 1) buf[i++] = *str++;
				break;
			}
			case 'd': {
				int num = va_arg(args, int);
				itoa(num, num_buf, 10);
				for (size_t j = 0; num_buf[j] != '\0' && i < buf_size - 1;
					 j++) {
					buf[i++] = num_buf[j];
				}
				break;
			}
			case 'X': {
				unsigned int num = va_arg(args, unsigned int);
				itoa(num, num_buf, 16);
				for (size_t j = 0; num_buf[j] != '\0' && i < buf_size - 1;
					 j++) {
					buf[i++] = num_buf[j];
				}
				break;
			}
			case 'c': {
				buf[i++] = (char)va_arg(args, int);
				break;
			}
			case 'p': {
				unsigned long p = (unsigned long)va_arg(args, void*);
				if (i < buf_size - 1) buf[i++] = '0';
				if (i < buf_size - 1) buf[i++] = 'x';

				itoa(p, num_buf, 16);
				for (size_t j = 0; num_buf[j] != '\0' && i < buf_size - 1;
					 j++) {
					buf[i++] = num_buf[j];
				}
				break;
			}
			case '%': {
				buf[i++] = '%';
				break;
			}
			default: {
				buf[i++] = *ptr;
				break;
			}
		}
	}
	buf[i] = '\0';
	return i;
}
