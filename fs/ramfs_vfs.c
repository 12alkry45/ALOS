#include <arch/memory.h>
#include <fs/ramfs_vfs.h>
#include <lib/mem.h>

ramfs_info_t* ramfs_info = NULL;

static vnode_t* create_vnode(vfs_t* mount_point, tree_node_t* node);

int read(vnode_t* node, void* buffer, size_t size, uint32_t offset);
int write(vnode_t* node, const void* data, size_t size, uint32_t offset);
int lookup(vnode_t* node_dir, const char* name, vnode_t** result);

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
	root->flags = VNODE_NONE;
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
