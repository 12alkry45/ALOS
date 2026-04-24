#ifndef MM_FRAME_H
#define MM_FRAME_H

#include <stddef.h>
#include <stdint.h>

void init_frames(uint32_t size);
void clear_frame(uint32_t frame_addr);
uint32_t alloc_frame();

#endif
