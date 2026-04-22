#ifndef MM_FRAME_H
#define MM_FRAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_frames(uintptr_t* addr, size_t memory_size);
uintptr_t alloc_frame();
void free_frame(uintptr_t frame_addr);
bool frame_is_free(uintptr_t frame_addr);
uintptr_t get_first_free_frame();
size_t frame_get_total();

#endif
