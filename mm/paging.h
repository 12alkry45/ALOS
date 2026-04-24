#ifndef MM_PAGING_H
#define MM_PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t present : 1;
	uint32_t writeable : 1;
	uint32_t usermode : 1;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t reserved : 7;
	uint32_t frame_addr : 20;
} page_t;

typedef struct {
	page_t pages[1024];
} page_table_t;

typedef struct {
	page_table_t* virtual_table[1024];
	uint32_t physical_table[1024];
	uint32_t physical_addr;
} page_directory_t;

void init_paging();
void switch_page_directory(page_directory_t* dir);
page_t* get_page(uint32_t virt_addr, int create_table, page_directory_t* dir);

#endif
