#include <arch/memory.h>
#include <arch/timer.h>
#include <fs/ramfs.h>
#include <lib/mem.h>

tree_node_t* ramfs_root_node = NULL;

tree_node_t* create_node(tree_node_t* node, char* name, node_type_t type);

tree_node_t* ramfs_init_internal() {
	tree_node_t* root_fs = (tree_node_t*)kmalloc(sizeof(tree_node_t));
	root_fs->child = root_fs->parent = root_fs->sibling = NULL;
	root_fs->meta.type = NODE_DIR;
	memcpy(root_fs->meta.name, "/", 2); 
	root_fs->meta.size = 0;
	root_fs->data = NULL;
	
	uint32_t cur_time = get_tick();
	root_fs->meta.create_time = cur_time;
	root_fs->meta.access_time = cur_time;
	root_fs->meta.modify_time = cur_time;

	tree_node_t* dev  = create_node(root_fs, "dev", NODE_DIR);
	tree_node_t* etc  = create_node(root_fs, "etc", NODE_DIR);
	tree_node_t* root = create_node(root_fs, "root", NODE_DIR);
	tree_node_t* home = create_node(root_fs, "home", NODE_DIR);
	tree_node_t* user = create_node(home, "user", NODE_DIR);
	
	tree_node_t* readme = create_node(root, "readme.txt", NODE_FILE);
	if (readme) {
		const char* txt = "Welcome to ALOS Kernel!\n";
		ramfs_write(readme, (void*)txt, strlen(txt), 0);
	}

	tree_node_t* hostname = create_node(etc, "hostname", NODE_FILE);
	if (hostname) {
		const char* txt = "alos\n";
		ramfs_write(hostname, (void*)txt, strlen(txt), 0);
	}

	tree_node_t* hello = create_node(user, "hello.txt", NODE_FILE);
	if (hello) {
		const char* txt = "Hello, world! You're here.\n";
		ramfs_write(hello, (void*)txt, strlen(txt), 0);
	}

	ramfs_root_node = root_fs;
	return ramfs_root_node;
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

tree_node_t* ramfs_get_root_node() { return ramfs_root_node; }

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

static char* get_next_part(char* path, char** part, size_t* len) {
	if (path == NULL || *path != '/' || path[1] == '\0') return NULL;
	if (*path == '/') path++;
	*part = path;
	*len = 0;

	while (*path != '\0' && *path != '/') {
		(*len)++;
		path++;
	}
	return path;
}

tree_node_t* lookup_path(char* abs_path) {
	if (!abs_path || abs_path[0] != '/') return NULL;
	tree_node_t* cur_node = ramfs_get_root_node();
	if (!cur_node) return NULL;

	char* cur_path = abs_path;
	char* comp = NULL;
	size_t comp_len = 0;

	while ((cur_path = get_next_part(cur_path, &comp, &comp_len)) != NULL) {
		if (comp_len > MAX_NAME_LENGTH) return NULL;
		char buf[comp_len + 1];
		memcpy(buf, comp, comp_len);
		buf[comp_len] = '\0';
		tree_node_t* next_node = ramfs_lookup(cur_node, buf);
		if (!next_node) return NULL;
		cur_node = next_node;
	}
	return cur_node;
}

void ramfs_detach_node(tree_node_t* node) {
	if (!node || !node->parent) return;
	tree_node_t* parent = node->parent;
	if (parent->child == node) {
		parent->child = node->sibling;
		node->sibling = NULL;
		node->parent = NULL;
		return;
	}
	tree_node_t* prev = parent->child;
	while (prev && prev->sibling != node) {
		prev = prev->sibling;
	}
	if (prev) {
		prev->sibling = node->sibling;
		node->sibling = NULL;
		node->parent = NULL;
	}
}

void ramfs_attach_node(tree_node_t* parent, tree_node_t* node) {
	if (!parent || !node) return;
	if (parent->meta.type != NODE_DIR) return;
	if (node->parent != NULL) return;
	node->parent = parent;
	node->sibling = parent->child;
	parent->child = node;
}

static void ramfs_destroy_tree(tree_node_t* node) {
	if (!node) return;
	tree_node_t* child = node->child;
	while (child) {
		tree_node_t* next = child->sibling;
		ramfs_destroy_tree(child);
		child = next;
	}
	if (node->data) {
		kfree(node->data);
		node->data = NULL;
	}
	kfree(node);
}

void ramfs_remove_node(tree_node_t* node) {
	if (!node) return;
	if (node->parent == NULL) return;  // only unmount
	if (node->meta.type == NODE_DIR && node->child != NULL) return;

	ramfs_detach_node(node);
	if (node->meta.type == NODE_FILE && node->data) {
		kfree(node->data);
		node->data = NULL;
	}
	kfree(node);
}

void ramfs_remove_recursive(tree_node_t* node) {
	if (!node) return;
	if (node->parent == NULL) return;

	tree_node_t* child = node->child;
    while (child) {
        tree_node_t* next = child->sibling;
        ramfs_destroy_tree(child);
        child = next;
    }
    node->child = NULL;
    ramfs_detach_node(node);
	if (node->data) {
		kfree(node->data);
		node->data = NULL;
	}
	kfree(node);
}
