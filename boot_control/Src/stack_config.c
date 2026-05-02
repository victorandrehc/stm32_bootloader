#include "stack_config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"   // or f1xx, l4xx, etc.


#define U32_TO_PTR(U32, PTR_TYPE) ((PTR_TYPE*)&(U32))
extern uint32_t _sstack;
extern uint32_t _estack;

#define STACK_PAINT_PATTERN 0x55

void print_stack(void)
{
    printf("_estack: %p\t_sstack: %p\tsize:0x%x\tused:0x%x\n", U32_TO_PTR(_estack, void), U32_TO_PTR(_sstack, void), U32_TO_PTR(_estack, uint8_t) - U32_TO_PTR(_sstack, uint8_t), get_stack_usage());
}


void init_stack(void)
{
    uint8_t* pivot = U32_TO_PTR(_sstack, uint8_t);
    uint8_t* stack_bottom = U32_TO_PTR(_estack, uint8_t);
    while(pivot < stack_bottom)
    {
        *pivot = STACK_PAINT_PATTERN;
        pivot++;
    }
}

__attribute__((optimize("O0")))
size_t get_stack_usage(void)
{
   uint8_t* pivot = U32_TO_PTR(_sstack, uint8_t);
    uint8_t* stack_bottom = U32_TO_PTR(_estack, uint8_t);
    printf("_sstack:%p\t_estack:%p\n",pivot, stack_bottom);
    while(pivot < stack_bottom)
    {
       printf("pivot: %p\tvalue 0x%x\tdiff: %x\n", pivot, *pivot, pivot - stack_bottom);
        pivot++;
    }
    return pivot - stack_bottom;

} 