#include "keyboard.h"

#include "../arch/isr.h"
#include "../arch/ports.h"
#include "../kernel/kernel.h"
#include "../lib/function.h"
#include "../lib/string.h"
#include "screen.h"

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define SCANCODE_MAX 57

static char key_buffer[256];

const char* scancode_name[] = {
	"ERROR", "Esc",	 "1",	   "2",	 "3",	  "4",	   "5",			"6",
	"7",	 "8",	 "9",	   "0",	 "-",	  "=",	   "Backspace", "Tab",
	"Q",	 "W",	 "E",	   "R",	 "T",	  "Y",	   "U",			"I",
	"O",	 "P",	 "[",	   "]",	 "Enter", "Lctrl", "A",			"S",
	"D",	 "F",	 "G",	   "H",	 "J",	  "K",	   "L",			";",
	"'",	 "`",	 "LShift", "\\", "Z",	  "X",	   "C",			"V",
	"B",	 "N",	 "M",	   ",",	 ".",	  "/",	   "RShift",	"Keypad *",
	"LAlt",	 "Space"};

const char scancode_ascii[] = {
	'?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',	'0', '-', '=',	'?',
	'?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',	'[', ']', '?',	'?',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z',
	'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '?', '?',	'?', ' '};

static void keyboard_callback(registers_t* r) {
	uint8_t scancode = port_byte_in(PORT_KEYBOARD_READ);
	if (scancode > SCANCODE_MAX) return;
	if (scancode == BACKSPACE) {
		backspace(key_buffer);
		kernel_print_backspace();
	} else if (scancode == ENTER) {
		kernel_print("\n");
		user_input(key_buffer);
		key_buffer[0] = '\0';
	} else {
		char letter = scancode_ascii[(int)scancode];
		char str[2] = {letter, '\0'};
		append(key_buffer, letter);
		kernel_print(str);
	}
	UNUSED(r);
}

void init_keyboard() { register_interrupt_handler(IRQ1, keyboard_callback); }
