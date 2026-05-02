#pragma once
#include <stddef.h>

void   init_stack(void);
void   print_stack_info(void);
void   print_heap_info(void);

size_t get_stack_size(void);
size_t get_stack_high_water(void);
void traverse_stack(void);

size_t get_max_heap_available(void);
size_t get_max_heap_reserved(void);
