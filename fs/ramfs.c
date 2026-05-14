#include <arch/memory.h>
#include <arch/timer.h>
#include <fs/ramfs.h>
#include <lib/mem.h>

ramfs_info_t* ramfs_info = NULL;

tree_node_t* create_node(tree_node_t* node, char* name, node_type_t type);

void ramfs_init() {
	ramfs_info = (ramfs_info_t*)kmalloc(sizeof(ramfs_info_t));
	tree_node_t* root_fs = (tree_node_t*)kmalloc(sizeof(tree_node_t));
	root_fs->child = root_fs->parent = root_fs->sibling = NULL;
	root_fs->meta.type = NODE_DIR;

	create_node(root_fs, "dev", NODE_DIR);
	tree_node_t* home = create_node(root_fs, "home", NODE_DIR);
	create_node(home, "hello.txt", NODE_FILE);

	ramfs_info->root_node = root_fs;
}

tree_node_t* ramfs_lookup(tree_node_t* dir, const char* name) {
	if (!dir || dir->meta.type != NODE_DIR) return NULL;
	dir->meta.access_time = get_tick();
	tree_node_t* cur = dir->child;
	while (cur) {
		if (strcmp(name, cur->meta.name) == 0) {
			cur->meta.access_time = get_tick();
			return cur;
		}
		cur = cur->sibling;
	}
	return NULL;
}

size_t ramfs_read(tree_node_t* file, void* buffer, uint32_t size,
				  uint32_t offset) {
	if (!file || file->meta.type != NODE_FILE || !buffer) return 0;
	file->meta.access_time = get_tick();
	if (file->meta.size < offset) return 0;
	uint32_t read_threshold =
		(size + offset > file->meta.size) ? file->meta.size - offset : size;
	memcpy(buffer, (uint8_t*)file->data + offset, read_threshold);
	return read_threshold;
}

size_t ramfs_write(tree_node_t* file, void* data, uint32_t size,
				   uint32_t offset) {
	if (!file || file->meta.type != NODE_FILE) return 0;
	uint32_t new_size =
		(size + offset > file->meta.size) ? size + offset : file->meta.size;
	if (new_size > file->meta.size || !file->data) {
		kfree(file->data);
		void* new_data = kmalloc(new_size);
		if (!new_data) return 0;
		file->data = new_data;
	}
	if (data && size > 0) memcpy((uint8_t*)file->data + offset, data, size);
	file->meta.size = new_size;
	uint32_t cur_time = get_tick();
	file->meta.modify_time = cur_time;
	file->meta.access_time = cur_time;
	return size;
}

tree_node_t* ramfs_get_root_node() { return ramfs_info->root_node; }

tree_node_t* create_node(tree_node_t* parent, char* name, node_type_t type) {
	tree_node_t* node = (tree_node_t*)kmalloc(sizeof(tree_node_t));
	node->meta.type = type;
	memcpy(node->meta.name, name, MAX_NAME_LENGTH - 1);
	node->meta.name[MAX_NAME_LENGTH - 1] = '\0';
	node->meta.size = 0;
	node->data = NULL;
	uint32_t cur_time = get_tick();
	node->meta.access_time = cur_time;
	node->meta.modify_time = cur_time;
	node->meta.create_time = cur_time;
	node->parent = parent;
	node->child = NULL;
	if (parent->child == NULL) {
		parent->child = node;
	} else {
		tree_node_t* cur = parent->child;
		while (cur->sibling) cur = cur->sibling;
		cur->sibling = node;
	}
	return node;
}
