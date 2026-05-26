#include <arch/memory.h>
#include <arch/timer.h>
#include <fs/ramfs_vfs.h>
#include <lib/mem.h>

ramfs_info_t* ramfs_info = NULL;

static vnode_t* create_vnode(vfs_t* mount_point, tree_node_t* node);

int read(vnode_t* node, void* buffer, size_t size, uint32_t offset);
int write(vnode_t* node, const void* data, size_t size, uint32_t offset);
int lookup(vnode_t* node_dir, const char* name, vnode_t** result);

int create(vnode_t* dir, const char* name, uint32_t mode, vnode_t** result);
int remove(vnode_t* dir, const char* name);
int rename(vnode_t* old_dir, const char* old_name, vnode_t* new_dir,
		   const char* new_name);
int mkdir(vnode_t* dir, const char* name, uint32_t mode);
int rmdir(vnode_t* dir, const char* name);
int readdir(vnode_t* dir, uint32_t index, char* name_buffer,
			uint32_t name_buffer_size);

int mount_ramfs(vfs_t* mount_point);
int unmount_ramfs(vfs_t* mount_point);
int get_root_ramfs(vfs_t* mount_point, struct vnode** result);

filesystem_t ramfs_op = {
	.get_root = get_root_ramfs,
	.mount = mount_ramfs,
	.unmount = unmount_ramfs,
};

vnode_op_t ramfs_vnode_op = {
	.lookup = lookup,
	.read = read,
	.write = write,

	.create = create,
	.remove = remove,
	.rename = rename,
	.mkdir = mkdir,
	.rmdir = rmdir,
	.readdir = readdir,
};

void ramfs_init() {
	memcpy(ramfs_op.fs_name, "ramfs", 6);
	vfs_register(&ramfs_op);
}

int mount_ramfs(vfs_t* mount_point) {
	ramfs_info = (ramfs_info_t*)kmalloc(sizeof(ramfs_info_t));
	for (size_t i = 0; i < VNODE_MAX_NUM; i++) ramfs_info->vnode[i] = NULL;

	ramfs_info->root_node = ramfs_init_internal();

	vnode_t* root = (vnode_t*)kmalloc(sizeof(vnode_t));
	ramfs_info->root_vnode = root;
	root->flags = VNODE_ROOT;
	root->mounted_vfs = NULL;
	root->ref = 0;
	root->vnode_op = &ramfs_vnode_op;
	root->cur_vfs = mount_point;
	root->vnode_type = VDIR;
	root->vnode_spec_data = ramfs_info->root_node;

	mount_point->vfs_priv_data = ramfs_info;
	return VFS_OK;
}

int unmount_ramfs(vfs_t* mount_point) {
	ramfs_info_t* ramfs_info = (ramfs_info_t*)mount_point->vfs_priv_data;
	for (size_t i = 0; i < VNODE_MAX_NUM; i++) {
		if (ramfs_info->vnode[i] != NULL) kfree(ramfs_info->vnode[i]);
	}
	kfree(ramfs_info->root_vnode);
	kfree(ramfs_info);
	return VFS_OK;
}

int get_root_ramfs(vfs_t* mount_point, struct vnode** result) {
	ramfs_info_t* ramfs_info = (ramfs_info_t*)mount_point->vfs_priv_data;
	*result = ramfs_info->root_vnode;
	return VFS_OK;
}

int read(vnode_t* node, void* buffer, size_t size, uint32_t offset) {
	tree_node_t* file = (tree_node_t*)node->vnode_spec_data;
	return ramfs_read(file, buffer, size, offset);
}

int write(vnode_t* node, const void* data, size_t size, uint32_t offset) {
	tree_node_t* file = (tree_node_t*)node->vnode_spec_data;
	return ramfs_write(file, data, size, offset);
}

int lookup(vnode_t* node_dir, const char* name, vnode_t** result) {
	tree_node_t* dir = (tree_node_t*)node_dir->vnode_spec_data;
	tree_node_t* file = ramfs_lookup(dir, name);
	if (file == NULL) return VFS_ENOENT;
	*result = create_vnode(node_dir->cur_vfs, file);
	if (*result == NULL)
		return VFS_ERROR;
	else
		return VFS_OK;
}

int create(vnode_t* dir, const char* name, uint32_t mode, vnode_t** result) {
	if (!dir || !name || !result) return VFS_EINVAL;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;

	tree_node_t* dir_vnode = (tree_node_t*)dir->vnode_spec_data;
	if (!dir_vnode) return VFS_ERROR;

	tree_node_t* exist = ramfs_lookup(dir_vnode, name);
	if (exist) return VFS_EEXIST;

	tree_node_t* new_node = create_node(dir_vnode, (char*)name, NODE_FILE);
	if (!new_node) return VFS_ERROR;

	new_node->meta.mask = mode;
	new_node->meta.user_id = 0;
	new_node->meta.group_id = 0;
	new_node->meta.size = 0;
	new_node->meta.create_time = get_tick();
	new_node->meta.modify_time = new_node->meta.create_time;
	new_node->meta.access_time = new_node->meta.create_time;

	vnode_t* vnode = create_vnode(dir->cur_vfs, new_node);
	if (!vnode) return VFS_ERROR;

	*result = vnode;
	return VFS_OK;
}

int remove(vnode_t* dir, const char* name) {
	if (!dir || !name) return VFS_EINVAL;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;

	tree_node_t* dir_vnode = (tree_node_t*)dir->vnode_spec_data;
	if (!dir_vnode) return VFS_ERROR;

	tree_node_t* rm_node = ramfs_lookup(dir_vnode, name);
	if (!rm_node) return VFS_ENOENT;
	if (rm_node->meta.type == NODE_DIR) return VFS_EISDIR;

	ramfs_remove_node(rm_node);
	return VFS_OK;
}

int rename(vnode_t* old_dir, const char* old_name, vnode_t* new_dir,
		   const char* new_name) {
	if (!old_dir || !old_name || !new_dir || !new_name) return VFS_EINVAL;

	tree_node_t* old_parent = (tree_node_t*)old_dir->vnode_spec_data;
	tree_node_t* new_parent = (tree_node_t*)new_dir->vnode_spec_data;

	tree_node_t* rn_node = ramfs_lookup(old_parent, old_name);
	if (!rn_node) return VFS_ENOENT;

	tree_node_t* exist = ramfs_lookup(new_parent, new_name);
	if (exist) {
		if (exist->meta.type == NODE_FILE) {
			ramfs_remove_node(exist);
		} else {
			return VFS_EEXIST;
		}
	}

	memcpy(rn_node->meta.name, new_name, MAX_NAME_LENGTH);
	rn_node->meta.name[MAX_NAME_LENGTH - 1] = '\0';

	if (old_parent != new_parent) {
		ramfs_detach_node(rn_node);
		ramfs_attach_node(new_parent, rn_node);
	}
	return VFS_OK;
}

int mkdir(vnode_t* dir, const char* name, uint32_t mode) {
	if (!dir || !name) return VFS_EINVAL;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;

	tree_node_t* dir_vnode = (tree_node_t*)dir->vnode_spec_data;
	if (!dir_vnode) return VFS_ERROR;
	tree_node_t* exist = ramfs_lookup(dir_vnode, name);
	if (exist) return VFS_EEXIST;

	tree_node_t* new_dir = create_node(dir_vnode, (char*)name, NODE_DIR);
	if (!new_dir) return VFS_ERROR;

	new_dir->meta.mask = mode;
	new_dir->meta.user_id = 0;
	new_dir->meta.group_id = 0;
	new_dir->meta.size = 0;
	uint32_t now = get_tick();
	new_dir->meta.create_time = now;
	new_dir->meta.modify_time = now;
	new_dir->meta.access_time = now;
	return VFS_OK;
}

int rmdir(vnode_t* dir, const char* name) {
	if (!dir || !name) return VFS_EINVAL;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;
	tree_node_t* dir_vnode = (tree_node_t*)dir->vnode_spec_data;
	if (!dir_vnode) return VFS_ERROR;
	tree_node_t* rm_node = ramfs_lookup(dir_vnode, name);
	if (!rm_node) return VFS_ENOENT;
	if (rm_node->meta.type != NODE_DIR) return VFS_ENOTDIR;
	if (rm_node->child != NULL) return VFS_ENOTEMPTY;
	if (rm_node == ramfs_get_root_node()) return VFS_ERROR;
	ramfs_remove_node(rm_node);
	return VFS_OK;
}

int readdir(vnode_t* dir, uint32_t index, char* name_buffer,
			uint32_t name_buffer_size) {
	if (!dir || !name_buffer) return VFS_EINVAL;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;

	tree_node_t* dir_node = (tree_node_t*)dir->vnode_spec_data;
	if (!dir_node) return VFS_ERROR;

	tree_node_t* child = dir_node->child;
	for (uint32_t i = 0; i < index && child != NULL; i++) {
		child = child->sibling;
	}
	if (!child) return VFS_ENOENT;
	uint32_t name_len = strlen(child->meta.name);
	if (name_len >= name_buffer_size) return VFS_ERROR;
	memcpy(name_buffer, child->meta.name, name_len + 1);
	return VFS_OK;
}

static vnode_t* create_vnode(vfs_t* mount_point, tree_node_t* node) {
	ramfs_info_t* ramfs_info = (ramfs_info_t*)mount_point->vfs_priv_data;
	for (size_t i = 0; i < VNODE_MAX_NUM; i++) {
		if (ramfs_info->vnode[i] != NULL &&
			ramfs_info->vnode[i]->vnode_spec_data == (void*)node) {
			return ramfs_info->vnode[i];
		}
	}
	vnode_t* vnode = (vnode_t*)kmalloc(sizeof(vnode_t));
	vnode->flags = VNODE_NONE;
	vnode->mounted_vfs = NULL;
	vnode->ref = 0;
	vnode->vnode_op = &ramfs_vnode_op;
	vnode->cur_vfs = mount_point;
	vnode->vnode_spec_data = node;

	switch (node->meta.type) {
		case NODE_DIR:
			vnode->vnode_type = VDIR;
			break;
		case NODE_FILE:
			vnode->vnode_type = VREG;
			break;
		default:
			vnode->vnode_type = VNON;
			break;
	}

	for (size_t i = 0; i < VNODE_MAX_NUM; i++) {
		if (ramfs_info->vnode[i] == NULL) {
			ramfs_info->vnode[i] = vnode;
			return vnode;
		}
		if (ramfs_info->vnode[i]->ref <= 0) {
			kfree(ramfs_info->vnode[i]);
			ramfs_info->vnode[i] = vnode;
			return vnode;
		}
	}
	kfree(vnode);
	return NULL;
}
