#ifndef MM_FRAME_H
#define MM_FRAME_H

#include <stddef.h>
#include <stdint.h>

#define MEM_END_PAGE 0x1000000
#define MEM_SIZE (MEM_END_PAGE >> 12);

void init_frames();
void clear_frame(uint32_t frame_addr);
uint32_t alloc_frame();

#endif
