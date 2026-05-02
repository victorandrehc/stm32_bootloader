#include "stack_config.h"

#include "boot_config.h"
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
        printf("offset_words [%u] can't exceed stack size in words [%u]\n", (unsigned) offset_words, (unsigned) stack_size_words);
        return;
    }

    uint32_t* start = (uint32_t*) &_sstack + offset_words;
    const size_t remaining_size_bytes = (size_t) ((const uint8_t*) &_estack - (const uint8_t*) start);
    const size_t copy_bytes = remaining_size_bytes > STACK_USAGE_BYTES ? STACK_USAGE_BYTES : remaining_size_bytes;
    if (remaining_size_bytes > STACK_USAGE_BYTES)
    {
        printf("WARN: Traversing only the first %u bytes of the stack from the offset\n", (unsigned) copy_bytes);
    }

    printf("start: %p offset_words: %u[0x%x], remaining_size: %u[0x%x]\n",
           (const void*) start,
           (unsigned) offset_words,
           (unsigned) offset_words,
           (unsigned) copy_bytes,
           (unsigned) copy_bytes);
    memcpy(stack_copy, start, copy_bytes);

    const size_t n_words = copy_bytes / sizeof(uint32_t);
    const uint32_t* base = start;
    for (size_t i = 0; i < n_words; i++)
    {
        printf("addr: %p\tvalue: 0x%08lx\n", (const void*) (base + i), (unsigned long) stack_copy[i]);
    }
}

void hardfault_c(uint32_t* fault_sp)
{
    volatile crash_dump_t* cd = &bootloader_api_ptr->boot_info.crash_dump;
    cd->sp_at_fault = (uint32_t) fault_sp;
    cd->cfsr = SCB->CFSR;
    cd->hfsr = SCB->HFSR;
    cd->mmfar = SCB->MMFAR;
    cd->bfar = SCB->BFAR;

    /* Hardware-stacked exception frame: R0-R3, R12, LR, PC, xPSR */
    for (size_t i = 0; i < 8; i++)
    {
        cd->hw_frame[i] = fault_sp[i];
    }
    /* Capture from current SP up toward _estack, clipped to buffer */
    const uint8_t* top = (const uint8_t*) &_estack;
    size_t available = (size_t) (top - (const uint8_t*) fault_sp);
    if (available > CRASH_DUMP_STACK_BYTES)
    {
        available = CRASH_DUMP_STACK_BYTES;
    }

    cd->stack_captured = available;
    memcpy((void*) cd->stack, fault_sp, available);

    bootloader_api_ptr->reset(HARD_FAULT);
}

void crash_dump_print(void)
{
    if (bootloader_api_ptr->boot_info.reset_reason_uint != HARD_FAULT)
    {
        return;
    }
    volatile crash_dump_t* cd = &bootloader_api_ptr->boot_info.crash_dump;

    printf("=== CRASH DUMP BEGIN ===\n");
    printf("sp_at_fault: 0x%08lx\n", (unsigned long) cd->sp_at_fault);
    printf("CFSR=0x%08lx HFSR=0x%08lx MMFAR=0x%08lx BFAR=0x%08lx\n",
           (unsigned long) cd->cfsr,
           (unsigned long) cd->hfsr,
           (unsigned long) cd->mmfar,
           (unsigned long) cd->bfar);
    printf("hw frame: r0=%08lx r1=%08lx r2=%08lx r3=%08lx\n",
           (unsigned long) cd->hw_frame[0],
           (unsigned long) cd->hw_frame[1],
           (unsigned long) cd->hw_frame[2],
           (unsigned long) cd->hw_frame[3]);
    printf("          r12=%08lx lr=%08lx pc=%08lx xpsr=%08lx\n",
           (unsigned long) cd->hw_frame[4],
           (unsigned long) cd->hw_frame[5],
           (unsigned long) cd->hw_frame[6],
           (unsigned long) cd->hw_frame[7]);
    printf("stack: %u bytes from 0x%08lx\n", (unsigned) cd->stack_captured, (unsigned long) cd->sp_at_fault);

    const size_t n = cd->stack_captured / sizeof(uint32_t);
    for (size_t i = 0; i < n; i++)
    {
        printf("  [%02u] 0x%08lx: 0x%08lx\n",
               (unsigned) i,
               (unsigned long) (cd->sp_at_fault + i * sizeof(uint32_t)),
               (unsigned long) cd->stack[i]);
    }
    printf("=== CRASH DUMP END ===\n");
}
