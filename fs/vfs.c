#include <arch/memory.h>
#include <fs/vfs.h>
#include <lib/mem.h>
#include <lib/string.h>
#include <vfs_errno.h>

vfs_t* vfs_root;
filesystem_t* registered_fs[VFS_MAX_FS];
size_t number_fs;
vfs_file_t vfs_open_file[MAX_OPEN_FILES];

static vnode_t* lookup_path(const char* path);

void vfs_init() {
	vfs_root = NULL;
	number_fs = 0;
	for (size_t i = 0; i < VFS_MAX_FS; i++) registered_fs[i] = NULL;
	for (size_t i = 0; i < MAX_OPEN_FILES; i++) vfs_open_file[i].vnode = NULL;
}

void vfs_register(filesystem_t* fs) {
	if (number_fs >= VFS_MAX_FS) return;
	registered_fs[number_fs++] = fs;
}

static filesystem_t* find_filesystem(const char* name) {
	for (size_t i = 0; i < number_fs; i++) {
		if (strcmp(registered_fs[i]->fs_name, name) == 0) {
			return registered_fs[i];
		}
	}
	return NULL;
}

vfs_error_t vfs_mount(const char* fs_name, const char* mount_point) {
	filesystem_t* fs = find_filesystem(fs_name);
	if (fs == NULL) return VFS_ERROR;
	vfs_t* vfs_node = kmalloc(sizeof(vfs_t));
	vfs_node->next_fs = NULL;
	vfs_node->vfs_op = fs;

	if (vfs_root == NULL)
		vfs_node->mount_point = NULL;
	else {
		vfs_node->mount_point = lookup_path(mount_point);
		if (vfs_node->mount_point == NULL ||
			(vfs_node->mount_point->flags & VNODE_ROOT) == VNODE_ROOT) {
			free(vfs_node);
			return VFS_ENOENT;
		}
		if (vfs_node->mount_point->vnode_type != VDIR) {
			free(vfs_node);
			return VFS_ENOTDIR;
		}
		vfs_node->mount_point->ref++;
		vfs_node->mount_point->mounted_vfs = vfs_node;
	}
	vfs_node->vfs_op->mount(vfs_node);

	if (vfs_root == NULL) vfs_root = vfs_node;
	vfs_t* cur = vfs_root;
	while (cur->next_fs != NULL) cur = cur->next_fs;
	cur->next_fs = vfs_node;

	return VFS_OK;
}

vfs_error_t vfs_unmount(const char* mount_point) {
	vnode_t* vnode = lookup_path(mount_point);
	if (vnode == NULL) return VFS_ENOENT;
	if ((vnode->flags & VNODE_ROOT) != VNODE_ROOT) return VFS_ERROR;
	vfs_t* vfs_node = vnode->cur_vfs;
	if (vfs_root == vfs_node) return VFS_EACCESS;
	vfs_node->mount_point->mounted_vfs = NULL;
	vfs_node->mount_point->ref--;

	vfs_node->vfs_op->unmount(vfs_node);

	vfs_t* cur = vfs_root;
	while (cur->next_fs != vfs_node) cur = cur->next_fs;
	cur->next_fs = NULL;

	free(vfs_node);
	return VFS_OK;
}

fd_t vfs_open(const char* path, uint16_t mode);
int vfs_close(fd_t descriptor);

size_t vfs_read(fd_t fd, void* buffer, size_t size);
size_t vfs_write(fd_t fd, const void* data, size_t size);

static vnode_t* lookup_path(const char* path) {
	if (!path || path[0] != '/') return NULL;

	vnode_t* vnode_res = NULL;
	char path_copy[VFS_MAX_PATH_LENGTH];
	memcpy(path_copy, path, VFS_MAX_PATH_LENGTH);

	vfs_root->vfs_op->get_root(vfs_root, &vnode_res);
	char* position = path_copy;
	char name[VFS_MAX_PATH_LENGTH];

	while (vnode_res != NULL && *position != '\0') {
		while (*position == '/') position++;
		if (*position == '\0') break;

		char* end_pos = position;
		while (*end_pos != '\0' && *end_pos != '/') end_pos++;
		size_t len = end_pos - position;

		if (len >= VFS_MAX_PATH_LENGTH) return NULL;
		memcpy(name, position, len);
		name[len] = '\0';

		if (vnode_res->mounted_vfs != NULL) {
			vnode_res->mounted_vfs->vfs_op->get_root(vnode_res->mounted_vfs,
													 &vnode_res);
		}
		if (vnode_res != NULL) {
			vnode_res->vnode_op->lookup(vnode_res, name, &vnode_res);
		}

		position = end_pos;
	}

	if (vnode_res != NULL && vnode_res->mounted_vfs != NULL) {
		vnode_res->mounted_vfs->vfs_op->get_root(vnode_res->mounted_vfs,
												 &vnode_res);
	}

	return vnode_res;
}
