#include "../drivers/screen.h"
#include "utils.h"

void main() {
	clear_screen();

	for (int i = 0; i < 25; i++) {
		char str[255];
		int_to_ascii(i, str);
		kernel_print_at(str, 0, i);
	}

	kernel_print_at(
		"This text forces the kernel to scroll. Row 0 will disappear. ", 60,
		24);
}
