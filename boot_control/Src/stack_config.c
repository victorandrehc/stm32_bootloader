#include "stack_config.h"
#include <stdio.h>
#include <stdint.h>
#define U32_TO_PTR(U32, PTR_TYPE) ((PTR_TYPE*)&(U32))
extern uint32_t _sstack;
extern uint32_t _estack;

#define STACK_PAINT_PATTERN 0xAFAF


void print_stack()
{
    printf("_estack: %p\t_sstack: %p\tsize:0x%x\n", U32_TO_PTR(_estack, void), U32_TO_PTR(_sstack, void), U32_TO_PTR(_estack, uint8_t) - U32_TO_PTR(_sstack, uint8_t));
}


void init_stack()
{
    size_t* stack_begin = U32_TO_PTR(_estack, size_t);
    size_t* stack_end = U32_TO_PTR(_sstack, size_t);
    while(stack_begin<stack_end)
    {
        *stack_begin = STACK_PAINT_PATTERN
        stack_begin++;
    }
}