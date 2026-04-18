#include "screen.h"

#include "../cpu/ports.h"
#include "../lib/mem.h"

static int32_t get_cursor_offset();
static void set_cursor_offset(int32_t offset);
static int32_t get_offset_row();
static int32_t get_offset_col();
static int32_t get_offset(int32_t col, int32_t row);
static int32_t print_char(char c, int32_t col, int32_t row, uint8_t attr);

#define BACKSPACE 0x08

/**
 * Print the message on the specified location.
 * If col or row is negative, we will use current offset.
 */
void kernel_print_at(char* message, int32_t col, int32_t row) {
	int32_t offset = 0;
	if (col >= 0 && row >= 0) {
		offset = get_offset(col, row);
	} else {
		offset = get_cursor_offset();
		col = get_offset_col(offset);
		row = get_offset_row(offset);
	}

	for (int i = 0; message[i] != 0; ++i) {
		offset = print_char(message[i], col, row, WHITE_ON_BLACK);
		col = get_offset_col(offset);
		row = get_offset_row(offset);
	}
}

/**
 * Print the message on the current cursor location
 */
void kernel_print(char* message) { kernel_print_at(message, -1, -1); }

/**
 * Clear the screen
 */
void clear_screen() {
	uint8_t* video_memory = (uint8_t*)VIDEO_MEMORY;
	int screen_size = MAX_COLS * MAX_ROWS;
	for (int i = 0; i < screen_size; ++i) {
		video_memory[i * 2] = ' ';
		video_memory[i * 2 + 1] = WHITE_ON_BLACK;
	}
	set_cursor_offset(0);
}

void kernel_print_backspace() {
	int32_t offset = get_cursor_offset() - 2;
	int32_t col = get_offset_col(offset);
	int32_t row = get_offset_row(offset);
	print_char(BACKSPACE, col, row, WHITE_ON_BLACK);	 // backspace
}

/**
 * Print function for the kernel, directly accesses the video memory
 *
 * If 'col' and 'row' are negative, we will print at current cursor location
 * If 'attr' is zero it will use 'white on black' as default
 * Returns the offset of the next character
 * Sets the video cursor to the returned offset
 */
static int32_t print_char(char c, int32_t col, int32_t row, uint8_t attr) {
	uint8_t* video_memory = (uint8_t*)VIDEO_MEMORY;
	if (col >= MAX_COLS || row >= MAX_ROWS) {
		video_memory[2 * MAX_ROWS * MAX_COLS - 2] = 'E';
		video_memory[2 * MAX_ROWS * MAX_COLS - 1] = RED_ON_WHITE;
		return get_offset(col, row);
	}

	if (!attr) attr = WHITE_ON_BLACK;

	int32_t offset = 0;
	if (col >= 0 && row >= 0) {
		offset = get_offset(col, row);
	} else {
		offset = get_cursor_offset();
	}

	if (c == '\n') {
		row = get_offset_row(offset);
		offset = get_offset(0, row + 1);
	} else if (c == BACKSPACE) {
		video_memory[offset] = ' ';
		video_memory[offset + 1] = attr;
	} else {
		video_memory[offset] = c;
		video_memory[offset + 1] = attr;
		offset += 2;
	}

	if (offset >= MAX_COLS * MAX_ROWS * 2) {
		memory_copy((uint8_t*)(get_offset(0, 1) + VIDEO_MEMORY),
					(uint8_t*)(get_offset(0, 0) + VIDEO_MEMORY),
					2 * MAX_COLS * (MAX_ROWS - 1));
		char* last_line = (char*)(get_offset(0, MAX_ROWS - 1) + VIDEO_MEMORY);
		for (int i = 0; i < MAX_COLS * 2; ++i) last_line[i] = 0;
		offset -= 2 * MAX_COLS;
	}

	set_cursor_offset(offset);
	return offset;
}

static int32_t get_cursor_offset() {
	port_byte_out(REG_SCREEN_CTRL, 14);
	int32_t offset = port_byte_in(REG_SCREEN_DATA) << 8;
	port_byte_out(REG_SCREEN_CTRL, 15);
	offset += port_byte_in(REG_SCREEN_DATA);
	return offset * 2;
}

static void set_cursor_offset(int32_t offset) {
	offset >>= 1;
	port_byte_out(REG_SCREEN_CTRL, 14);
	port_byte_out(REG_SCREEN_DATA, offset >> 8);
	port_byte_out(REG_SCREEN_CTRL, 15);
	port_byte_out(REG_SCREEN_DATA, offset & 0xFF);
}

static int32_t get_offset(int32_t col, int32_t row) {
	return 2 * (row * MAX_COLS + col);
}

static int32_t get_offset_row(int32_t offset) {
	return offset / (2 * MAX_COLS);
}

static int32_t get_offset_col(int32_t offset) {
	return (offset - (get_offset_row(offset) * 2 * MAX_COLS)) / 2;
}
