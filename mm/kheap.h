#ifndef KHEAP_H
#define KHEAP_H

#include <lib/ordered_array.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KHEAP_START 0xC0000000
#define KHEAP_INITIALISE_SIZE 0x100000
#define HEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIC_NUM 0x6789ABCD
#define HEAP_MIN_SIZE 0x70000
#define PAGE_SIZE 0x1000

typedef struct {
	uint32_t check;
	bool is_hole;
	size_t size;
} header_t;

typedef struct {
	uint32_t check;
	header_t* header;
} footer_t;

typedef struct {
	ordered_array_t index;
	uint32_t start_address;
	uint32_t end_address;
	uint32_t max_address;
	bool supervisor;
	bool readonly;
} heap_t;

heap_t* create_heap(uint32_t start, uint32_t end, uint32_t max, bool supervisor,
					bool readonly);
void* alloc(size_t size, bool align, heap_t* heap);
void free(void* page, heap_t* heap);

#endif
