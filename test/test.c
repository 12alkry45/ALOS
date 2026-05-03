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
