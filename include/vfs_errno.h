#ifndef VFS_ERRNO_H
#define VFS_ERRNO_H

typedef enum {
	VFS_OK = 0,			  // Success
	VFS_EPERM = -1,		  // Permission error
	VFS_ENOENT = -2,	  // File or dir not found
	VFS_EBADF = -9,		  // Bad descriptor
	VFS_EACCESS = -13,	  // Permission denied
	VFS_EEXIST = -17,	  // Already exist
	VFS_ENOTDIR = -20,	  // Error not a dir
	VFS_EISDIR = -21,	  // Error is a dir
	VFS_EINVAL = -22,	  // Invalid arg
	VFS_ENFILE = -23,	  // Too many open files
	VFS_ENOTEMPTY = -39,  // Dir not empty
	VFS_ERROR = -100,	  // Generic error
} vfs_error_t;

#endif
