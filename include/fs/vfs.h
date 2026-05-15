#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

#define VFS_MAX_PATH_LENGTH 256
#define VFS_MAX_FILENAME 64

typedef enum { O_RDONLY, O_WRONLY, O_RDWR } vfs_open_mode_t;
typedef enum { VNODE_NONE, VNODE_ROOT } vnode_flags_t;

struct filesystem;
struct vnode;
struct vnode_op;

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
	vnode_op_t* vnode_op;

	vtype_t vnode_type;
	uint32_t ref;
	void* vnode_spec_data;
} vnode_t;

typedef struct vnode_op {
	int (*read)(vnode_t* node, void* buffer, size_t size, uint32_t offset);
	int (*write)(vnode_t* node, const void* data, size_t size, uint32_t offset);
	int (*lookup)(vnode_t* node_dir, const char* name, vnode_t** result);
} vnode_op_t;

typedef struct filesystem {
	char fs_name[VFS_MAX_FILENAME];
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

void vfs_init();
void vfs_register(filesystem_t* fs);

int vfs_mount(const char* vfs_name, const char* mount_point);
int vfs_unmount(const char* mount_point);

fd_t vfs_open(const char* path, uint16_t mode);
int vfs_close(fd_t descriptor);

size_t vfs_read(fd_t fd, void* buffer, size_t size);
size_t vfs_write(fd_t fd, const void* data, size_t size);

#endif
