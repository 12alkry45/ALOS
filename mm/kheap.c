#include "kheap.h"

#include <lib/function.h>
#include <lib/mem.h>

#include "mm/paging.h"

heap_t* kheap;
extern page_directory_t* kernel_directory;

static int32_t find_smallest_hole(size_t size, bool align, heap_t* heap);
static int8_t less_than_header_t(void* a, void* b);
static void expand_heap(size_t new_size, heap_t* heap);
static uint32_t contract_heap(size_t new_size, heap_t* heap);

heap_t* create_heap(uint32_t start, uint32_t end, uint32_t max, bool supervisor,
					bool readonly) {
	heap_t* heap = (heap_t*)kmalloc(sizeof(heap_t));
	ASSERT(start % PAGE_SIZE == 0);
	ASSERT(end % PAGE_SIZE == 0);
	heap->index =
		place_ordered_array((void*)start, HEAP_INDEX_SIZE, &less_than_header_t);
	start += HEAP_INDEX_SIZE * sizeof(type_t);
	if ((start & 0xFFF) != 0) {
		start &= 0xFFFFF000;
		start += 0x1000;
	}
	heap->start_address = start;
	heap->end_address = end;
	heap->max_address = max;
	heap->supervisor = supervisor;
	heap->readonly = readonly;

	header_t* hole = (header_t*)start;
	hole->check = HEAP_MAGIC_NUM;
	hole->is_hole = true;
	hole->size = end - start;
	insert_ordered_array((void*)hole, &heap->index);
	return heap;
}

void* alloc(size_t size, bool align, heap_t* heap) {
	size_t new_size = size + sizeof(header_t) + sizeof(footer_t);
	int32_t iterator = find_smallest_hole(new_size, align, heap);
	if (iterator == -1) {
		int32_t old_length = heap->end_address - heap->start_address;
		int32_t old_end_addr = heap->end_address;
		expand_heap(old_length + new_size, heap);
		int32_t new_length = heap->end_address - heap->start_address;
		iterator = 0;
		uint32_t idx = -1, value = 0x0;
		while (iterator < heap->index.cur_size) {
			uint32_t cur_block =
				(uint32_t)look_up_ordered_array(iterator, &heap->index);
			if (cur_block > value) {
				value = cur_block;
				idx = iterator;
			}
			iterator++;
		}
		if (idx == -1) {
			header_t* header = (header_t*)old_end_addr;
			header->check = HEAP_MAGIC_NUM;
			header->size = new_length - old_length;
			header->is_hole = true;
			footer_t* footer =
				(footer_t*)(old_end_addr + header->size - sizeof(footer_t));
			footer->check = HEAP_MAGIC_NUM;
			footer->header = header;
			insert_ordered_array((void*)header, &heap->index);
		} else {
			header_t* header = look_up_ordered_array(idx, &heap->index);
			header->size += new_length - old_length;
			footer_t* footer =
				(footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
			footer->header = header;
			footer->check = HEAP_MAGIC_NUM;
		}
		return alloc(size, align, heap);
	}
	header_t* orig_hole_header =
		(header_t*)look_up_ordered_array(iterator, &heap->index);
	uint32_t orig_hole_addr = (uint32_t)orig_hole_header;
	uint32_t orig_hole_size = orig_hole_header->size;
	if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t)) {
		size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}

	if (align && (orig_hole_addr & 0xFFF)) {
		uint32_t new_addr = orig_hole_addr + PAGE_SIZE -
							(orig_hole_addr & 0xFFF) - sizeof(header_t);
		header_t* hole_header = (header_t*)orig_hole_addr;
		hole_header->check = HEAP_MAGIC_NUM;
		hole_header->is_hole = true;
		hole_header->size =
			PAGE_SIZE - (orig_hole_addr & 0xFFF) - sizeof(header_t);
		footer_t* hole_footer =
			(footer_t*)((uint32_t)new_addr - sizeof(footer_t));
		hole_footer->check = HEAP_MAGIC_NUM;
		hole_footer->header = hole_header;
		orig_hole_addr = new_addr;
		orig_hole_size = orig_hole_size - hole_header->size;
	} else {
		remove_ordered_array(iterator, &heap->index);
	}

	header_t* block_header = (header_t*)orig_hole_addr;
	block_header->check = HEAP_MAGIC_NUM;
	block_header->is_hole = false;
	block_header->size = new_size;
	footer_t* block_footer =
		(footer_t*)(orig_hole_addr + sizeof(header_t) + size);
	block_footer->check = HEAP_MAGIC_NUM;
	block_footer->header = block_header;

	if (orig_hole_size - new_size > 0) {
		header_t* hole_header = (header_t*)(orig_hole_addr + sizeof(header_t) +
											size + sizeof(footer_t));
		hole_header->check = HEAP_MAGIC_NUM;
		hole_header->is_hole = true;
		hole_header->size = orig_hole_size - new_size;
		footer_t* hole_footer =
			(footer_t*)((uint32_t)hole_header + orig_hole_size - new_size -
						sizeof(footer_t));
		if ((uint32_t)hole_footer < heap->end_address) {
			hole_footer->check = HEAP_MAGIC_NUM;
			hole_footer->header = hole_header;
		}
		insert_ordered_array((void*)hole_header, &heap->index);
	}
	return (void*)((uint32_t)block_header + sizeof(header_t));
}

void free(void* p, heap_t* heap) {
	if (p == 0) return;
	header_t* header = (header_t*)((uint32_t)p - sizeof(header_t));
	footer_t* footer =
		(footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
	ASSERT(header->check == HEAP_MAGIC_NUM);
	ASSERT(footer->check == HEAP_MAGIC_NUM);
	header->is_hole = true;
	bool do_add = true;
	// unify left
	footer_t* test_footer;
	if ((uint32_t)header > heap->start_address) {
		test_footer = (footer_t*)((uint32_t)header - sizeof(footer_t));
		if (test_footer->check == HEAP_MAGIC_NUM &&
			test_footer->header->is_hole) {
			uint32_t cache_size = header->size;
			header = test_footer->header;
			footer->header = header;
			header->size += cache_size;
			do_add = false;
		}
	}
	// unify right
	header_t* test_header = (header_t*)((uint32_t)footer + sizeof(footer_t));
	if ((uint32_t)test_header < heap->end_address &&
		test_header->check == HEAP_MAGIC_NUM && test_header->is_hole) {
		header->size += test_header->size;
		test_footer = (footer_t*)((uint32_t)test_header + test_header->size -
								  sizeof(footer_t));
		footer = test_footer;
		uint32_t iterator = 0;
		while ((iterator < heap->index.cur_size) &&
			   (look_up_ordered_array(iterator, &heap->index) !=
				(void*)test_header)) {
			iterator++;
		}
		ASSERT(iterator < heap->index.cur_size);
		remove_ordered_array(iterator, &heap->index);
	}
	/*if ((uint32_t)footer + sizeof(footer_t) == heap->end_address) {
		uint32_t old_length = heap->end_address - heap->start_address;
		uint32_t new_length =
			contract_heap((uint32_t)header - heap->start_address, heap);
		if (header->size - (old_length - new_length) > 0) {
			header->size -= old_length - new_length;
			footer =
				(footer_t*)((uint32_t)header + header->size - sizeof(footer_t));
			footer->header = header;
		} else {
			uint32_t iterator = 0;
			while ((iterator < heap->index.cur_size) &&
				   (look_up_ordered_array(iterator, &heap->index) !=
					(void*)test_header)) {
				iterator++;
			}
			if (iterator < heap->index.cur_size) {
				remove_ordered_array(iterator, &heap->index);
			}
		}
	}*/
	if (do_add) insert_ordered_array((void*)header, &heap->index);
}

static void expand_heap(size_t new_size, heap_t* heap) {
	ASSERT(new_size > heap->start_address - heap->end_address);
	if ((new_size & 0xFFF) != 0) {
		new_size &= 0xFFFFF000;
		new_size += 0x1000;
	}
	ASSERT(heap->start_address + new_size < heap->max_address);
	size_t old_size = heap->end_address - heap->start_address;
	size_t addr = heap->start_address + old_size;
	while (addr > heap->start_address + new_size) {
		alloc_page(get_page(addr, 1, kernel_directory),
				   (heap->supervisor) ? 1 : 0, (heap->readonly) ? 1 : 0);
		addr += PAGE_SIZE;
	}
	heap->end_address = heap->start_address + new_size;
}

static uint32_t contract_heap(size_t new_size, heap_t* heap) {
	ASSERT(new_size < heap->start_address - heap->end_address);
	if ((new_size & 0xFFF) != 0) {
		new_size &= 0xFFFFF000;
		new_size += 0x1000;
	}
	if (new_size < HEAP_MIN_SIZE) new_size = HEAP_MIN_SIZE;
	size_t old_size = heap->end_address - heap->start_address;
	size_t addr = heap->start_address + old_size - 0x1000;
	while (addr > heap->start_address + new_size) {
		free_page(get_page(addr, 0, kernel_directory));
		addr -= PAGE_SIZE;
	}
	heap->end_address = heap->start_address + new_size;
}

static int8_t less_than_header_t(void* a, void* b) {
	return (((header_t*)a)->size < ((header_t*)b)->size) ? 1 : 0;
}

static int32_t find_smallest_hole(size_t size, bool align, heap_t* heap) {
	uint32_t iterator = 0;
	while (iterator < heap->index.cur_size) {
		header_t* header =
			(header_t*)look_up_ordered_array(iterator, &heap->index);
		if (align) {
			uint32_t location = (uint32_t)header;
			int32_t offset = 0;
			if (((location + sizeof(header)) & 0xFFF) != 0) {
				offset = PAGE_SIZE - (location + sizeof(header_t)) % PAGE_SIZE;
			}
			size_t hole_size = header->size - offset;
			if (hole_size >= size) {
				break;
			}
		} else if (header->size >= size) {
			break;
		}
		iterator++;
	}
	return (iterator == heap->index.cur_size) ? -1 : iterator;
}
