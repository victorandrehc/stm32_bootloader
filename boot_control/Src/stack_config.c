#include "stack_config.h"

#include "stm32f4xx_hal.h" /* for __get_MSP() via CMSIS */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint32_t _sstack;
extern uint32_t _estack;

extern uint32_t _sheap;
extern uint32_t _eheap;

#define STACK_PAINT_WORD          0xDEADBEEFu
#define PAINT_SAFETY_MARGIN_BYTES 64u /* headroom so we don't clobber our own frame */
#define STACK_USAGE_BYTES         0x1000
#define STACK_USAGE_WORDS         (STACK_USAGE_BYTES / sizeof(uint32_t))

void init_stack(void)
{
    uint32_t* p = &_sstack;
    uintptr_t sp = (uintptr_t) __get_MSP();
    uint32_t* end = (uint32_t*) ((sp - PAINT_SAFETY_MARGIN_BYTES) & ~(uintptr_t) 3u);

    if (end > &_estack)
        end = &_estack;
    while (p < end)
    {
        *p++ = STACK_PAINT_WORD;
    }
}

size_t get_stack_size(void)
{
    return (size_t) ((const uint8_t*) &_estack - (const uint8_t*) &_sstack);
}

size_t get_max_heap_available(void)
{
    return (size_t) ((const uint8_t*) &_sstack - (const uint8_t*) &_sheap);
}

size_t get_max_heap_reserved(void)
{
    return (size_t) ((const uint8_t*) &_eheap - (const uint8_t*) &_sheap);
}

size_t get_stack_high_water(void)
{
    const uint32_t* p = &_sstack;
    const uint32_t* e = &_estack;

    while (p < e && *p == STACK_PAINT_WORD)
    {
        p++;
    }
    return (size_t) ((const uint8_t*) e - (const uint8_t*) p);
}

void print_stack_info(void)
{
    size_t stack_size = get_stack_size();
    size_t stack_usage = get_stack_high_water();
    unsigned stack_usage_pct = stack_size ? (unsigned) ((stack_usage * 100u) / stack_size) : 0u;

    printf("stack: base=%p top=%p size=%u high_water=%u (%u%%)\n",
           (void*) &_sstack,
           (void*) &_estack,
           (unsigned) stack_size,
           (unsigned) stack_usage,
           stack_usage_pct);
}

void print_heap_info(void)
{
    size_t max_available = get_max_heap_available();
    size_t reserved = get_max_heap_reserved();

    printf("heap: base=%p top=%p max available=%u reserved=%u\n",
           (void*) &_sheap,
           (void*) &_eheap,
           (unsigned) max_available,
           (unsigned) reserved);
}

static uint32_t stack_copy[STACK_USAGE_WORDS];
void traverse_stack(void)
{
    traverse_stack_skip_words(0);
}

void traverse_stack_skip_words(size_t offset_words)
{
    const size_t stack_size_words = get_stack_size() / sizeof(uint32_t);
    if (offset_words >= stack_size_words)
    {
        printf("offset_words [%u] cant go over the stack size in words %u", offset_words, stack_size_words);
        return;
    }

    uint32_t* start = (uint32_t*) &_sstack + offset_words;
    const size_t remaining_size_bytes = (size_t) ((const uint8_t*) &_estack - (const uint8_t*) start);
    const size_t copy_bytes = remaining_size_bytes > STACK_USAGE_BYTES ? STACK_USAGE_BYTES : remaining_size_bytes;
    if (remaining_size_bytes > STACK_USAGE_BYTES)
    {
        printf("WARN: Traversing only the first %u bytes of the stack from the offset", (unsigned) copy_bytes);
    }

    printf("start: %p offset_words: %u[0x%x], reaminaing_size: %u[0x%x]\n",
           (const void*) start,
           offset_words,
           offset_words,
           copy_bytes,
           copy_bytes);
    memcpy(stack_copy, start, copy_bytes);

    const size_t n_words = copy_bytes / sizeof(uint32_t);
    const uint32_t* base = start;
    for (size_t i = 0; i < n_words; i++)
    {
        printf("addr: %p\tvalue: 0x%08lx\n", (const void*) (base + i), (unsigned long) stack_copy[i]);
    }
}
