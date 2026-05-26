#include <arch/memory.h>
#include <fs/vfs.h>
#include <lib/mem.h>
#include <lib/string.h>
#include <stdbool.h>

vfs_t* vfs_root;
filesystem_t* registered_fs[VFS_MAX_FS];
size_t number_fs;
vfs_file_t vfs_open_file[MAX_OPEN_FILES];

static vnode_t* lookup_path(const char* path);
static vfs_error_t split_path(const char* path, char* dir, char* name);

void vfs_init() {
	vfs_root = NULL;
	number_fs = 0;
	for (size_t i = 0; i < VFS_MAX_FS; i++) registered_fs[i] = NULL;
	for (size_t i = 0; i < MAX_OPEN_FILES; i++) vfs_open_file[i].vnode = NULL;
}

vfs_t* get_vfs_root() { return vfs_root; }

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
		vnode_t* mount_vnode = lookup_path(mount_point);
		vfs_node->mount_point = mount_vnode;
		if (mount_vnode == NULL ||
			(mount_vnode->flags & VNODE_ROOT) == VNODE_ROOT) {
			kfree(vfs_node);
			return VFS_ENOENT;
		}
		if (mount_vnode->vnode_type != VDIR) {
			kfree(vfs_node);
			return VFS_ENOTDIR;
		}
		mount_vnode->ref++;
		mount_vnode->mounted_vfs = vfs_node;
	}
	vfs_node->vfs_op->mount(vfs_node);

	if (vfs_root == NULL) {
		vfs_root = vfs_node;
	} else {
		vfs_t* cur = vfs_root;
		while (cur->next_fs != NULL) {
			cur = cur->next_fs;
		}
		cur->next_fs = vfs_node;
	}

	return VFS_OK;
}

vfs_error_t vfs_unmount(const char* mount_point) {
	vnode_t* mount_vnode = lookup_path(mount_point);
	if (mount_vnode == NULL) return VFS_ENOENT;
	if ((mount_vnode->flags & VNODE_ROOT) != VNODE_ROOT) return VFS_ERROR;
	vfs_t* vfs_node = mount_vnode->cur_vfs;

	if (vfs_root == vfs_node) return VFS_EACCESS;

	if (vfs_node->mount_point != NULL) {
		vfs_node->mount_point->mounted_vfs = NULL;
		vfs_node->mount_point->ref--;
	}

	vfs_node->vfs_op->unmount(vfs_node);

	vfs_t* cur = vfs_root;
	while (cur->next_fs != vfs_node && cur != NULL) cur = cur->next_fs;
	if (cur != NULL) cur->next_fs = vfs_node->next_fs;

	kfree(vfs_node);
	return VFS_OK;
}

static fd_t find_free_fd() {
	for (int i = 0; i < MAX_OPEN_FILES; i++) {
		if (vfs_open_file[i].vnode == NULL) return i;
	}
	return VFS_ENFILE;
}

fd_t vfs_open(const char* path, uint16_t mode) {
	vnode_t* file_node = lookup_path(path);
	if (file_node == NULL) return VFS_ENOENT;
	if (file_node->vnode_type != VREG) return VFS_EISDIR;

	fd_t descr = find_free_fd();
	if (descr == VFS_ENFILE) return VFS_ENFILE;

	file_node->ref++;
	vfs_open_file[descr].mode = mode;
	vfs_open_file[descr].vnode = file_node;
	vfs_open_file[descr].position = 0;
	return descr;
}

static bool is_fd_valid(fd_t fd) {
	if (fd < 0 || fd > MAX_OPEN_FILES) return false;
	if (vfs_open_file[fd].vnode == NULL) return false;
	return true;
}

vfs_error_t vfs_close(fd_t fd) {
	if (!is_fd_valid(fd)) return VFS_EBADF;
	vfs_open_file[fd].vnode->ref--;
	vfs_open_file[fd].vnode = NULL;
	return VFS_OK;
}

size_t vfs_read(fd_t fd, void* buffer, size_t size) {
	if (!is_fd_valid(fd)) return VFS_EBADF;
	vfs_file_t* open = &vfs_open_file[fd];
	if (open->mode != O_RDONLY && open->mode != O_RDWR) return VFS_EACCESS;
	int ret =
		open->vnode->vnode_op->read(open->vnode, buffer, size, open->position);
	if (ret < 0) return ret;
	open->position += ret;
	return ret;
}

size_t vfs_write(fd_t fd, const void* data, size_t size) {
	if (!is_fd_valid(fd)) return VFS_EBADF;
	vfs_file_t* open = &vfs_open_file[fd];
	if (open->mode != O_WRONLY && open->mode != O_RDWR) return VFS_EACCESS;
	int ret =
		open->vnode->vnode_op->write(open->vnode, data, size, open->position);
	if (ret < 0) return ret;
	open->position += ret;
	return ret;
}

vfs_error_t vfs_create(const char* path, uint32_t mode) {
	if (!path) return VFS_EINVAL;
	char dir_path[VFS_MAX_PATH_LENGTH];
	char file_name[VFS_MAX_FILENAME];
	if (split_path(path, dir_path, file_name) != VFS_OK) {
		return VFS_EINVAL;
	}
	vnode_t* dir = lookup_path(dir_path);
	if (!dir) return VFS_ENOENT;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;
	if (!dir->vnode_op->create) return VFS_ENOSYS;
	vnode_t* file = NULL;
	int status = dir->vnode_op->create(dir, file_name, mode, &file);
	if (file) file->ref--;
	return status;
}

vfs_error_t vfs_remove(const char* path) {
	if (!path) return VFS_EINVAL;

	char dir_path[VFS_MAX_PATH_LENGTH];
	char file_name[VFS_MAX_FILENAME];
	if (split_path(path, dir_path, file_name) != VFS_OK) {
		return VFS_EINVAL;
	}

	vnode_t* dir = lookup_path(dir_path);
	if (!dir) return VFS_ENOENT;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;
	if (!dir->vnode_op->remove) return VFS_ENOSYS;
	return dir->vnode_op->remove(dir, file_name);
}

vfs_error_t vfs_rename(const char* old_path, const char* new_path) {
	if (!old_path || !new_path) return VFS_EINVAL;

	char old_dir[VFS_MAX_PATH_LENGTH], new_dir[VFS_MAX_PATH_LENGTH];
	char old_file[VFS_MAX_FILENAME], new_file[VFS_MAX_FILENAME];
	if (split_path(old_path, old_dir, old_file) != VFS_OK) return VFS_EINVAL;
	if (split_path(new_path, new_dir, new_file) != VFS_OK) return VFS_EINVAL;

	vnode_t* old_dir_vnode = lookup_path(old_dir);
	vnode_t* new_dir_vnode = lookup_path(new_dir);
	if (!old_dir_vnode || !new_dir_vnode) return VFS_ENOENT;
	if (old_dir_vnode->vnode_type != VDIR ||
		new_dir_vnode->vnode_type != VDIR) {
		return VFS_ENOTDIR;
	}
	if (!old_dir_vnode->vnode_op->rename) return VFS_ENOSYS;
	return old_dir_vnode->vnode_op->rename(old_dir_vnode, old_file,
										   new_dir_vnode, new_file);
}

vfs_error_t vfs_mkdir(const char* path, uint32_t mode) {
	if (!path) return VFS_EINVAL;
	char dir_path[VFS_MAX_PATH_LENGTH], dir_name[VFS_MAX_FILENAME];
	if (split_path(path, dir_path, dir_name) != VFS_OK) return VFS_EINVAL;
	vnode_t* dir = lookup_path(dir_path);
	if (!dir) return VFS_ENOENT;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;
	if (!dir->vnode_op->mkdir) return VFS_ENOSYS;
	return dir->vnode_op->mkdir(dir, dir_name, mode);
}

vfs_error_t vfs_rmdir(const char* path) {
	if (!path) return VFS_EINVAL;
	char dir_path[VFS_MAX_PATH_LENGTH], dir_name[VFS_MAX_FILENAME];
	if (split_path(path, dir_path, dir_name) != VFS_OK) return VFS_EINVAL;
	vnode_t* dir = lookup_path(dir_path);
	if (!dir) return VFS_ENOENT;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;
	if (!dir->vnode_op->rmdir) return VFS_ENOSYS;
	return dir->vnode_op->rmdir(dir, dir_name);
}

vfs_error_t vfs_readdir(const char* path, uint32_t index, char* name_buffer,
						uint32_t name_buffer_size) {
	if (!path || !name_buffer) return VFS_EINVAL;

	vnode_t* dir = lookup_path(path);
	if (!dir) return VFS_ENOENT;
	if (dir->vnode_type != VDIR) return VFS_ENOTDIR;

	if (!dir->vnode_op->readdir) return VFS_ENOSYS;
	return dir->vnode_op->readdir(dir, index, name_buffer, name_buffer_size);
}

vfs_error_t vfs_getattr(const char* path, vfs_stat_t* stat_buf) {
	if (!path || !stat_buf) return VFS_EINVAL;

	vnode_t* node = lookup_path(path);
	if (!node) return VFS_ENOENT;

	if (!node->vnode_op->getattr) return VFS_ENOSYS;
	return node->vnode_op->getattr(node, stat_buf);
}

vfs_error_t vfs_setattr(const char* path, vfs_stat_t* stat_buf, uint32_t mask) {
	if (!path || !stat_buf) return VFS_EINVAL;

	vnode_t* node = lookup_path(path);
	if (!node) return VFS_ENOENT;

	if (!node->vnode_op->setattr) return VFS_ENOSYS;
	return node->vnode_op->setattr(node, stat_buf, mask);
}

vfs_error_t vfs_access(const char* path, uint32_t mode) {
	if (!path) return VFS_EINVAL;

	vnode_t* node = lookup_path(path);
	if (!node) return VFS_ENOENT;

	if (!node->vnode_op->access) return VFS_ENOSYS;
	return node->vnode_op->access(node, mode);
}

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
			vnode_t* next_node = NULL;
			int status =
				vnode_res->vnode_op->lookup(vnode_res, name, &next_node);
			if (status != VFS_OK || next_node == NULL) return NULL;
			vnode_res = next_node;
		}

		position = end_pos;
	}

	if (vnode_res != NULL && vnode_res->mounted_vfs != NULL) {
		vnode_res->mounted_vfs->vfs_op->get_root(vnode_res->mounted_vfs,
												 &vnode_res);
	}

	return vnode_res;
}

static vfs_error_t split_path(const char* path, char* dir, char* name) {
	if (!path || !dir || !name) return VFS_ERROR;

	const char* last_slash = NULL;
	for (const char* p = path; *p; p++) {
		if (*p == '/') last_slash = p;
	}

	if (!last_slash) return VFS_ERROR;

	size_t dir_len = last_slash - path;
	if (dir_len == 0) {
		dir[0] = '/';
		dir[1] = '\0';
	} else {
		if (dir_len >= VFS_MAX_PATH_LENGTH) return VFS_ERROR;
		memcpy(dir, path, dir_len);
		dir[dir_len] = '\0';
	}

	const char* name_start = last_slash + 1;
	if (*name_start == '\0') return VFS_ERROR;

	size_t name_len = strlen(name_start);
	if (name_len >= VFS_MAX_FILENAME) return VFS_ERROR;
	memcpy(name, name_start, name_len + 1);
	return VFS_OK;
}
