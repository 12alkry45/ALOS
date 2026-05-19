#ifndef TEST_H
#define TEST_H

#include <lib/stdio.h>
#include <stddef.h>
#include <stdint.h>

#define ASSERT_TEST(msg, condition)                           \
	do {                                                      \
		if (condition) {                                      \
			printf("[OK]: %s\n", msg);                        \
		} else {                                              \
			printf("[FAIL]: %s (Line: %d)\n", msg, __LINE__); \
			return;                                           \
		}                                                     \
	} while (0)

void test_mm();
void test_ramfs();
void test_ramfs_creation();
void test_vfs_layer();

#endif
