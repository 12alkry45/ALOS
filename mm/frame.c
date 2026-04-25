#include "frame.h"

#include <lib/bitmap.h>
#include <lib/mem.h>
#include <lib/panic.h>

bitmap_word_t* frames;
uint32_t frames_num;

static void set_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr / 0x1000;
	bitmap_set(frames, frame);
}

void clear_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr / 0x1000;
	bitmap_clear(frames, frame);
}

static uint32_t test_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr / 0x1000;
	return bitmap_test(frames, frame);
}

static uint32_t first_frame() {
	return (uint32_t)bitmap_find_first_zero(frames, frames_num);
}

static uint32_t get_bitmap_frames_size() {
	return BITMAP_WORDS(frames_num) * sizeof(bitmap_word_t);
}

void init_frames() {
	frames_num = MEM_SIZE;
	uint32_t bitmap_size = get_bitmap_frames_size();
	frames = (bitmap_word_t*)kmalloc(bitmap_size);
	memory_set((uint8_t*)frames, 0, bitmap_size);
}

uint32_t alloc_frame() {
	uint32_t idx = first_frame();
	if (idx == (uint32_t)-1) {
		PANIC("No free frames!");
	}
	bitmap_set(frames, idx);
	return idx;
}
