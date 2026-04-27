#ifndef ORDERED_ARRAY_H
#define ORDERED_ARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void* type_t;
typedef int8_t (*less_than_function_t)(type_t, type_t);
typedef struct {
	type_t* array;
	uint32_t cur_size;
	uint32_t max_size;
	less_than_function_t less_than;
} ordered_array_t;

int8_t standart_less_than_function(type_t a, type_t b);

ordered_array_t create_ordered_array(size_t max_size, less_than_function_t less_than);
ordered_array_t place_ordered_array(void* address, size_t max_size,
						 less_than_function_t less_than);

void destroy_ordered_array(ordered_array_t* array);
void insert_ordered_array(type_t item, ordered_array_t* array);
type_t look_up_ordered_array(size_t i, ordered_array_t* array);
void remove_ordered_array(size_t i, ordered_array_t* array);

#endif
