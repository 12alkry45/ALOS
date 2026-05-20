#include <arch/memory.h>
#include <fs/ramfs_vfs.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <test/test.h>

void test_vfs_layer() {
	printf("\n --- TEST SIMPLE VFS ---\n");

	vfs_init();
	ramfs_init();

	vfs_error_t mnt_err = vfs_mount("ramfs", "/");
	ASSERT_TEST("Mounting RAMFS to root '/'", mnt_err == VFS_OK);
	ASSERT_TEST("Checking if vfs_root is set", get_vfs_root() != NULL);

	fd_t fd_read = vfs_open("/home/user/hello.txt", O_RDONLY);
	ASSERT_TEST("Opening initialized file /home/user/hello.txt", fd_read >= 0);

	if (fd_read >= 0) {
		char read_buffer[64];
		memset(read_buffer, 0, sizeof(read_buffer));

		size_t bytes_read = vfs_read(fd_read, read_buffer, 13);
		ASSERT_TEST("Reading from file", bytes_read == 13);
		ASSERT_TEST("Verifying read data content",
					strcmp(read_buffer, "Hello, world!") == 0);

		size_t bad_write = vfs_write(fd_read, "CrashTest", 9);
		ASSERT_TEST("Preventing write to O_RDONLY file",
					(int)bad_write == VFS_EACCESS);

		vfs_error_t close_err = vfs_close(fd_read);
		ASSERT_TEST("Closing file descriptor", close_err == VFS_OK);
	}

	fd_t fd_write = vfs_open("/etc/hostname", O_RDWR);
	ASSERT_TEST("Opening configuration file /etc/hostname in O_RDWR",
				fd_write >= 0);

	if (fd_write >= 0) {
		char host_buffer[32];
		memset(host_buffer, 0, sizeof(host_buffer));

		size_t r1 = vfs_read(fd_write, host_buffer, 5);
		ASSERT_TEST("Reading default hostname", r1 == 5);

		extern vfs_file_t vfs_open_file[MAX_OPEN_FILES];
		ASSERT_TEST("Checking file position increment",
					vfs_open_file[fd_write].position == 5);

		const char* patch = "patched";
		size_t w1 = vfs_write(fd_write, patch, 7);
		ASSERT_TEST("Writing modifications into file", w1 == 7);
		ASSERT_TEST("Checking file position after write",
					vfs_open_file[fd_write].position == 12);

		vfs_close(fd_write);
	}

	fd_t fd_fake = vfs_open("/dev/non_exist", O_RDONLY);
	ASSERT_TEST("Opening non-existent path returns ENOENT",
				fd_fake == VFS_ENOENT);

	fd_t fd_dir = vfs_open("/home/user", O_RDONLY);
	ASSERT_TEST("Opening directory as file returns EISDIR",
				fd_dir == VFS_EISDIR);

	char dummy[5];
	size_t r_bad = vfs_read(99, dummy, 5);
	ASSERT_TEST("Reading from invalid FD returns EBADF",
				(int)r_bad == VFS_EBADF);

	vfs_error_t unmnt_err = vfs_unmount("/");
	ASSERT_TEST("Preventing unmounting of system root '/'",
				unmnt_err == VFS_EACCESS);

	printf("\n--- TEST END ---\n\n");
}
