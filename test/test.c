#include <arch/memory.h>
#include <fs/ramfs.h>
#include <lib/mem.h>
#include <lib/stdio.h>
#include <test/test.h>

#define PRINT_PTR(name, ptr) printf("  %s: %p\n", name, ptr)

void test_simple_alloc(void) {
	printf("--- Test: Simple Alloc ---\n");
	void* a = kmalloc(8);
	void* b = kmalloc(16);
	PRINT_PTR("a (8) ", a);
	PRINT_PTR("b (16)", b);

	kfree(a);
	kfree(b);
	printf("Simple alloc passed.\n\n");
}

void test_reuse(void) {
	printf("--- Test: Reuse ---\n");
	void* a = kmalloc(100);
	void* b = kmalloc(100);
	void* c = kmalloc(100);

	PRINT_PTR("a", a);
	PRINT_PTR("b", b);
	PRINT_PTR("c", c);

	kfree(b);
	printf("Freed b\n");

	void* d = kmalloc(50);
	PRINT_PTR("d (50, should reuse b)", d);

	void* e = kmalloc(30);
	PRINT_PTR("e (30, after d)      ", e);

	kfree(a);
	kfree(c);
	kfree(d);
	kfree(e);
	printf("Reuse test passed.\n\n");
}

void test_ten_allocs(void) {
	printf("--- Test: Many Small Allocations ---\n");
	void* ptrs[10];

	for (int i = 0; i < 10; i++) {
		ptrs[i] = kmalloc(16);
		printf("p[%d]: %p\n", i, ptrs[i]);
	}

	for (int i = 0; i < 10; i += 2) {
		kfree(ptrs[i]);
	}
	printf("Freed even pointers\n");

	for (int i = 1; i < 10; i += 2) {
		kfree(ptrs[i]);
	}

	void* final = kmalloc(512);
	PRINT_PTR("Final block", final);
	kfree(final);

	printf("Freed odd pointers\n");
	printf("10 allocs test passed.\n\n");
}

void test_large_alloc(void) {
	printf("--- Test: Large Alloc ---\n");
	void* a = kmalloc(4096);
	void* b = kmalloc(1024 * 1024);

	PRINT_PTR("Large 1", a);
	PRINT_PTR("Large 2", b);

	kfree(a);
	kfree(b);
	printf("Large alloc passed.\n\n");
}

void test_mm() {
	test_simple_alloc();
	test_reuse();
	test_ten_allocs();
	test_large_alloc();
	printf("All memory tests passed!\n");
}

void test_ramfs() {
	printf("\n--- Start RAMFS Test ---\n");

	ramfs_init();
	printf("RAMFS initialized.\n");

	tree_node_t* root = ramfs_get_root_node();

	tree_node_t* home_dir = ramfs_lookup(root, "home");
	if (!home_dir || home_dir->meta.type != NODE_DIR) {
		printf("[FAIL] failed to find \"home\" directory!\n");
		return;
	}
	printf("[OK] ramfs_lookup successfully found \"home\" directory.\n");

	tree_node_t* hello_file = ramfs_lookup(home_dir, "hello.txt");
	if (!hello_file || hello_file->meta.type != NODE_FILE) {
		printf("[FAIL] failed to find \"hello.txt\"!\n");
		return;
	}
	printf("[OK] successfully found \"hello.txt\" file.\n");

	char* text1 = "Hello";
	size_t written = ramfs_write(hello_file, text1, 5, 0);
	if (written != 5 || hello_file->meta.size != 5) {
		printf("[FAIL] First write failed. Size: %d\n", hello_file->meta.size);
		return;
	}

	char* text2 = " ALOS!";
	written = ramfs_write(hello_file, text2, 6, 5);
	if (hello_file->meta.size != 11) {
		printf("[FAIL] Offset write failed. Total size: %d\n",
			   hello_file->meta.size);
		return;
	}
	printf("[OK] Data written with offset. File size: %d bytes.\n",
		   hello_file->meta.size);

	char read_buf[64];
	memset(read_buf, 0, sizeof(read_buf));

	size_t read_bytes = ramfs_read(hello_file, read_buf, 11, 0);
	read_buf[read_bytes] = '\0';

	if (read_bytes != 11) {
		printf("[FAIL] Read failed. Expected 11 bytes, got %d\n", read_bytes);
		return;
	}
	printf("[OK] Full read content: \"%s\"\n", read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	read_bytes = ramfs_read(hello_file, read_buf, 4, 6);
	read_buf[read_bytes] = '\0';

	printf("[OK] Substring read: \"%s\"\n", read_buf);
	printf("--- RAMFS Test Complete ---\n\n");
}

void test_ramfs_creation() {
	printf("\n--- Start RAMFS Creation Test ---\n");

	// /dev, /home, /home/hello.txt
	ramfs_init();
	printf("[OK] RAMFS initialized.\n");

	tree_node_t* root = ramfs_get_root_node();
	if (!root) {
		printf("[FAIL] Root node is NULL!\n");
		return;
	}

	tree_node_t* home_dir = ramfs_lookup(root, "home");
	if (!home_dir) {
		printf("[FAIL] Base /home directory not found!\n");
		return;
	}

	tree_node_t* user_dir = create_node(home_dir, "user1", NODE_DIR);
	if (!user_dir) {
		printf("[FAIL] Failed to create directory \"user1\"!\n");
		return;
	}
	printf("[OK] Directory \"user1\" created inside /home.\n");

	tree_node_t* config_file = create_node(user_dir, "config.ini", NODE_FILE);
	if (!config_file) {
		printf("[FAIL] Failed to create file \"config.ini\"!\n");
		return;
	}
	printf("[OK] File \"config.ini\" created inside /home/user1.\n");

	tree_node_t* verify_dir = ramfs_lookup(home_dir, "user1");
	tree_node_t* verify_file =
		(verify_dir) ? ramfs_lookup(verify_dir, "config.ini") : NULL;

	if (!verify_file || verify_file != config_file) {
		printf("[FAIL] Lookup system failed to find new structures!\n");
		return;
	}
	printf("[OK] Lookup successfully found file and dir.\n");

	char* init_payload = "theme=dark\nstatus=active\n";
	uint32_t payload_len = 25;

	size_t bytes_written =
		ramfs_write(config_file, init_payload, payload_len, 0);
	if (bytes_written != payload_len || config_file->meta.size != payload_len) {
		printf("[FAIL] Writing to new file failed. Size: %d\n",
			   config_file->meta.size);
		return;
	}
	printf("[OK] Successfully wrote %d bytes to config.ini.\n",
		   config_file->meta.size);

	char* modify_payload = "banned\n";
	size_t mod_written = ramfs_write(config_file, modify_payload, 7, 18);

	if (config_file->meta.size != 25) {
		printf("[FAIL] Offset modification altered size incorrectly: %d\n",
			   config_file->meta.size);
		return;
	}
	printf("[OK] Data modified with waited offset.\n");

	char read_buffer[64];
	memset(read_buffer, 0, sizeof(read_buffer));

	size_t bytes_read =
		ramfs_read(config_file, read_buffer, config_file->meta.size, 0);
	read_buffer[bytes_read] = '\0';

	if (bytes_read != 25) {
		printf("[FAIL] Reading updated file failed. Read bytes: %d\n",
			   bytes_read);
		return;
	}
	printf("[OK] Final file read successfully:\n---\n%s---\n", read_buffer);

	size_t out_of_bounds_read = ramfs_read(config_file, read_buffer, 10, 100);
	if (out_of_bounds_read != 0) {
		printf("[FAIL] Guard check failed: read allowed past EOF!\n");
		return;
	}
	printf("[OK] Guard check passed: reading past EOF safely returns 0.\n");

	printf("--- RAMFS Test End ---\n\n");
}
