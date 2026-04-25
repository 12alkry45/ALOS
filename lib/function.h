#ifndef FUNCTION_H
#define FUNCTION_H

#include "panic.h"

#define UNUSED(x) (void)(x);

#define ASSERT(condition)              \
	do {                               \
		if (!(condition)) {            \
			PANIC("Assertion failed!") \
		}                              \
	} while (0)

#endif
