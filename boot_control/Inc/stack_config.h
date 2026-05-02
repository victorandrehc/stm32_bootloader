#pragma once
#include <stddef.h>

void   init_stack(void);
void   print_stack(void);
size_t get_stack_size(void);
size_t get_stack_high_water(void);
size_t get_heap_size(void);
void traverse_stack(void);