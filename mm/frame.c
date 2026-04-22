#include "frame.h"

#include "../lib/bitmap.h"
#include "../lib/mem.h"
#include "../lib/panic.h"

static uint32_t* frame_bitmap = NULL;
static uintptr_t physical_memory_base = 0;
static size_t total_frames = 0;

void init_frames(uintptr_t* addr, size_t memory_size) {
	frame_bitmap = (uint32_t*)addr;
	total_frames = memory_size >> 12;
	memory_set((uint8_t*)frame_bitmap, 0,
			   BITMAP_WORDS(total_frames) * sizeof(uint32_t));
}

bool frame_is_free(uintptr_t frame_addr) {
	uint32_t frame_idx = frame_addr >> 12;
	if (frame_idx >= total_frames) return false;
	return bitmap_test((uint32_t*)frame_bitmap, frame_idx) == 0;
}

uintptr_t alloc_frame() {
	size_t frame_idx = bitmap_find_first_zero(frame_bitmap, total_frames);
	if (frame_idx == (size_t)-1) {
		PANIC("frame_alloc: out of memory");
	}
	bitmap_set(frame_bitmap, frame_idx);
	return (uintptr_t)frame_idx << 12;
}

void free_frame(uintptr_t frame_addr) {
	uint32_t frame_idx = frame_addr >> 12;
	if (frame_idx < total_frames) {
		bitmap_clear(frame_bitmap, frame_idx);
	}
}

uintptr_t get_first_free_frame() {
	return bitmap_find_first_zero(frame_bitmap, total_frames);
}

size_t frame_get_total() { return total_frames; }
