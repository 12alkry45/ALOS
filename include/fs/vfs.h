#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>
#include <vfs_errno.h>

#define VFS_MAX_PATH_LENGTH 256
#define VFS_MAX_FILENAME 64
#define VFS_FILESYSTEM_NAME 16
#define VFS_MAX_FS 8
#define MAX_OPEN_FILES 32

typedef enum { O_RDONLY, O_WRONLY, O_RDWR } vfs_open_mode_t;
typedef enum { VNODE_NONE, VNODE_ROOT } vnode_flags_t;

struct filesystem;
struct vnode;
struct vnode_op;
typedef struct stat;

typedef struct vfs {
	struct vfs* next_fs;
	struct filesystem* vfs_op;
	struct vnode* mount_point;
	void* vfs_priv_data;
} vfs_t;

typedef enum { VNON, VREG, VDIR } vtype_t;
typedef struct vnode {
	uint16_t flags;

	vfs_t* mounted_vfs;
	vfs_t* cur_vfs;
	struct vnode_op* vnode_op;

	vtype_t vnode_type;
	uint32_t ref;
	void* vnode_spec_data;
} vnode_t;

typedef struct vnode_op {
	int (*read)(vnode_t* node, void* buffer, size_t size, uint32_t offset);
	int (*write)(vnode_t* node, const void* data, size_t size, uint32_t offset);
	int (*lookup)(vnode_t* node_dir, const char* name, vnode_t** result);

	int (*getattr)(vnode_t* node, struct stat* stat_buf);
	int (*setattr)(vnode_t* node, struct stat* stat_buf, uint32_t mask);
	int (*access)(vnode_t* node, uint32_t mode);

	// Работа с файлами
	int (*create)(vnode_t* dir, const char* name, uint32_t mode,
				  vnode_t** result);
	int (*remove)(vnode_t* dir, const char* name);
	int (*rename)(vnode_t* old_dir, const char* old_name, vnode_t* new_dir,
				  const char* new_name);

	// Работа с директориями
	int (*mkdir)(vnode_t* dir, const char* name, uint32_t mode);
	int (*rmdir)(vnode_t* dir, const char* name);
	int (*readdir)(vnode_t* dir, uint32_t index, char* name_buffer,
				   uint32_t name_buffer_size);
} vnode_op_t;

typedef struct filesystem {
	char fs_name[VFS_FILESYSTEM_NAME];
	int (*mount)(vfs_t* mount_point);
	int (*unmount)(vfs_t* mount_point);
	int (*get_root)(vfs_t* mount_point, struct vnode** result);
} filesystem_t;

typedef int fd_t;

typedef struct vfs_file {
	vnode_t* vnode;
	uint16_t mode;
	uint32_t position;
} vfs_file_t;

typedef struct stat {
	uint32_t size;
	uint32_t mode;
	uint32_t uid;
	uint32_t gid;
	uint32_t atime;
	uint32_t mtime;
	uint32_t ctime;
} vfs_stat_t;

#define VFS_SETATTR_MODE 0x01
#define VFS_SETATTR_UID 0x02
#define VFS_SETATTR_GID 0x04
#define VFS_SETATTR_SIZE 0x08
#define VFS_SETATTR_ATIME 0x10
#define VFS_SETATTR_MTIME 0x20

void vfs_init();
void vfs_register(filesystem_t* fs);

vfs_error_t vfs_mount(const char* fs_name, const char* mount_point);
vfs_error_t vfs_unmount(const char* mount_point);

fd_t vfs_open(const char* path, uint16_t mode);
vfs_error_t vfs_close(fd_t descriptor);

size_t vfs_read(fd_t fd, void* buffer, size_t size);
size_t vfs_write(fd_t fd, const void* data, size_t size);

vfs_error_t vfs_create(const char* path, uint32_t mode);
vfs_error_t vfs_remove(const char* path);
vfs_error_t vfs_rename(const char* old_path, const char* new_path);
vfs_error_t vfs_mkdir(const char* path, uint32_t mode);
vfs_error_t vfs_rmdir(const char* path);
vfs_error_t vfs_readdir(const char* path, uint32_t index, char* name_buffer,
						uint32_t name_buffer_size);
vfs_error_t vfs_getattr(const char* path, vfs_stat_t* stat_buf);
vfs_error_t vfs_setattr(const char* path, vfs_stat_t* stat_buf, uint32_t mask);
vfs_error_t vfs_access(const char* path, uint32_t mode);

vfs_t* get_vfs_root();

#endif
