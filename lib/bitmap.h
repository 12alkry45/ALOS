#ifndef BITMAP_H
#define BITMAP_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t bitmap_word_t;
#define BITMAP_WORD_BITS (sizeof(bitmap_word_t) * 8)
#define BITMAP_WORDS(n) (((n) + BITMAP_WORD_BITS - 1) / (BITMAP_WORD_BITS))

#define INDEX_FROM_BIT(bit) ((bit) / (BITMAP_WORD_BITS))
#define OFFSET_FROM_BIT(bit) ((bit) % (BITMAP_WORD_BITS))

static inline void bitmap_set(bitmap_word_t* bitmap, size_t bit) {
	bitmap[INDEX_FROM_BIT(bit)] |= ((bitmap_word_t)1 << OFFSET_FROM_BIT(bit));
}

static inline void bitmap_clear(bitmap_word_t* bitmap, size_t bit) {
	bitmap[INDEX_FROM_BIT(bit)] &= ~((bitmap_word_t)1 << OFFSET_FROM_BIT(bit));
}

static inline bitmap_word_t bitmap_test(bitmap_word_t* bitmap, size_t bit) {
	return bitmap[INDEX_FROM_BIT(bit)] &
		   ((bitmap_word_t)1 << OFFSET_FROM_BIT(bit));
}

bitmap_word_t bitmap_find_first_zero(bitmap_word_t* bitmap, size_t bit_num);

#endif
