#include "ordered_array.h"

#include <arch/memory.h>

#include "function.h"
#include "lib/mem.h"

ordered_array_t create_ordered_array(size_t max_size,
									 less_than_function_t less_than) {
	ordered_array_t array;
	array.array = kmalloc(max_size * sizeof(type_t));
	memset(array.array, 0, max_size * sizeof(type_t));
	array.cur_size = 0;
	array.max_size = max_size;
	array.less_than = less_than;
	return array;
}

ordered_array_t place_ordered_array(void* address, size_t max_size,
									less_than_function_t less_than) {
	ordered_array_t array;
	array.array = (type_t*)address;
	memset(array.array, 0, max_size * sizeof(type_t));
	array.cur_size = 0;
	array.max_size = max_size;
	array.less_than = less_than;
	return array;
}

void insert_ordered_array(type_t item, ordered_array_t* array) {
	ASSERT(array->less_than);
	uint32_t iterator = 0;
	while (iterator < array->cur_size &&
		   array->less_than(array->array[iterator], item)) {
		iterator++;
	}
	ASSERT(array->cur_size < array->max_size);
	if (iterator == array->cur_size) {
		array->array[array->cur_size++] = item;
	} else {
		type_t buf_item = array->array[iterator];
		array->array[iterator] = buf_item;
		while (iterator < array->cur_size) {
			iterator++;
			type_t swap_item = array->array[iterator];
			array->array[iterator] = buf_item;
			buf_item = swap_item;
		}
		array->cur_size++;
	}
}

type_t look_up_ordered_array(size_t i, ordered_array_t* array) {
	ASSERT(i < array->cur_size);
	return array->array[i];
}

void remove_ordered_array(size_t i, ordered_array_t* array) {
	ASSERT(i < array->cur_size);
	while (i < array->cur_size) {
		array->array[i] = array->array[i + 1];
		++i;
	}
	array->cur_size--;
}

void destroy_ordered_array(ordered_array_t* array) { kfree(array->array); }

int8_t standart_less_than_function(type_t a, type_t b) {
	return (a < b) ? 1 : 0;
}
