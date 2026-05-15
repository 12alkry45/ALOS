#ifndef ERRNO_H
#define ERRNO_H

typedef enum {
	OK = 0,			  // Success
	EPERM = -1,		  // Permission error
	ENOENT = -2,	  // File or dir not found
	EBADF = -9,		  // Bad descriptor
	EACCES = -13,	  // Permission denied
	EEXIST = -17,	  // Already exist
	ENOTDIR = -20,	  // Error not a dir
	EISDIR = -21,	  // Error is a dir
	EINVAL = -22,	  // Invalid arg
	ENFILE = -23,	  // Too many open files
	ENOTEMPTY = -39,  // Dir not empty
	ERROR = -100,	  // Generic error
} vfs_error_t;

#endif
