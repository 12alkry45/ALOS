#ifndef RAMFS_H
#define RAMFS_H

#include <stddef.h>
#include <stdint.h>

#define MAX_NAME_LENGTH 64

typedef enum { NODE_FILE, NODE_DIR } node_type_t;

typedef struct metadata {
	char name[MAX_NAME_LENGTH];
	node_type_t type;
	uint32_t mask;
	uint32_t user_id;
	uint32_t group_id;
	uint32_t size;
	uint32_t create_time;
	uint32_t modify_time;
	uint32_t access_time;
} metadata_t;

typedef struct tree_node {
	metadata_t meta;
	struct tree_node* parent;
	struct tree_node* child;
	struct tree_node* sibling;
	void* data;
} tree_node_t;

void ramfs_init();
tree_node_t* ramfs_lookup(tree_node_t* dir, const char* name);
tree_node_t* ramfs_read(tree_node_t* file, void* data, uint32_t size,
						uint32_t offset);
tree_node_t* ramfs_write(tree_node_t* file, void* buffer, uint32_t size,
						 uint32_t offset);

#endif
