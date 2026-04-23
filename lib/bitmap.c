#include "bitmap.h"

size_t bitmap_find_first_zero(const bitmap_word_t* bitmap, size_t bit_num) {
	for (size_t i = 0; i < BITMAP_WORDS(bit_num); ++i) {
		if (bitmap[i] != (~(bitmap_word_t)0)) {
			for (size_t j = 0; j < BITMAP_WORD_BITS; ++j) {
				if (i * BITMAP_WORD_BITS + j >= bit_num) return (size_t)-1;
				if ((bitmap[i] & ((bitmap_word_t)1 << j)) == 0) {
					return i * BITMAP_WORD_BITS + j;
				}
			}
		}
	}
	return (size_t)-1;
}
