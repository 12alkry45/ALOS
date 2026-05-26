#ifndef RAMFS_VFS_H
#define RAMFS_VFS_H

#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <stddef.h>
#include <stdint.h>

#define VNODE_MAX_NUM 32

typedef struct {
	vnode_t* vnode[VNODE_MAX_NUM];
	vnode_t* root_vnode;
	tree_node_t* root_node;
} ramfs_info_t;

void ramfs_init();

#endif
