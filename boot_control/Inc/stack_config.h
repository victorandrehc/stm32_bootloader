#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * @file  stack_config.h
 * @brief Stack painting and diagnostics for runtime stack-usage analysis.
 *
 * The module paints the unused portion of the stack with a known sentinel
 * word (0xDEADBEEF) at boot, then computes the worst-case stack usage by
 * scanning for the first byte that no longer holds the sentinel. It also
 * exposes a few helpers for inspecting the linker-defined heap region.
 *
 * Required linker symbols (see app/stm32f401_app.ld):
 *   _sstack, _estack  -- bottom and top of the stack region
 *   _sheap,  _eheap   -- bottom and reserved-end of the heap region
 */

/**
 * @brief Paint the unused stack region with a sentinel pattern.
 *
 * Must be called once, very early in the boot path (typically from the
 * Reset_Handler before .data/.bss init). Reads the current MSP and paints
 * from `_sstack` up to a safe headroom below the live frame, so the
 * function never overwrites its own return address or locals.
 *
 * Calling this later than reset is supported but resets the watermark
 * baseline: any stack growth before the new call is forgotten.
 */
void init_stack(void);

/**
 * @brief Print stack base, top, total size, and high-water mark over UART.
 *
 * Output format:
 *   stack: base=0x... top=0x... size=N high_water=N (P%)
 *
 * Computes its measurements before invoking printf so the printf scratch
 * buffer cannot contaminate the reading.
 */
void print_stack_info(void);

/**
 * @brief Print heap base, ceiling, max-available, and reserved size.
 *
 * Output format:
 *   heap: base=0x... top=0x... max available=N reserved=N
 *
 * "max available" is the absolute upper bound the heap could ever reach
 * before colliding with the stack. "reserved" is the minimum guaranteed
 * by the linker (_Min_Heap_Size).
 */
void print_heap_info(void);

/**
 * @brief Return the total size of the stack region in bytes.
 *
 * Equivalent to `_estack - _sstack`, i.e. the value of `_Stack_Size`
 * configured in the linker script.
 *
 * @return Stack size in bytes.
 */
size_t get_stack_size(void);

/**
 * @brief Return the worst-case stack usage observed since the last paint.
 *
 * Scans the stack region from `_sstack` upward, counting words that still
 * hold the paint sentinel. The first non-paint word marks the deepest
 * point the stack pointer ever reached; the function returns the byte
 * distance from that point up to `_estack`.
 *
 * @return Bytes ever used (worst case). 0 means the stack has never been
 *         touched since `init_stack()`. A value equal to `get_stack_size()`
 *         indicates the stack was full at some point -- treat as overflow.
 *
 * @note Single-word sentinel matching has a small false-positive risk:
 *       a real stack word that happens to equal 0xDEADBEEF will terminate
 *       the scan early. In practice this is vanishingly rare.
 */
size_t get_stack_high_water(void);

/**
 * @brief Dump the contents of the stack region word by word over UART.
 *
 * Snapshots the stack into a static buffer first, then prints from the
 * snapshot. This avoids the printf-scratch-buffer contamination that
 * would corrupt the bottom of the live region.
 *
 * Output per line:
 *   addr: 0x...  byte_off: 0x... value: 0x........
 *
 * @note Buffer is sized for STACK_USAGE_BYTES (currently 0x1000). If the
 *       linker's `_Stack_Size` is larger, only the bottom portion is
 *       dumped. RAM is never overrun -- the copy is clamped.
 */
void traverse_stack(void);

/**
 * @brief Dump the stack region word-by-word starting `offset_words`
 *        words above `_sstack`.
 *
 * Snapshots the requested portion into the same static buffer used by
 * `traverse_stack` and prints from the snapshot, so the dump itself
 * does not contaminate the region being read. The printed range is
 * capped at STACK_USAGE_BYTES; if the request is larger, a `WARN`
 * line is emitted and only the first STACK_USAGE_BYTES are dumped.
 *
 * @param offset_words  Number of 32-bit words to skip from the bottom
 *                      of the stack. Must be strictly less than the
 *                      stack size in words; out-of-range values log a
 *                      message and return without dumping.
 */
void traverse_stack_skip_words(size_t offset_words);

/**
 * @brief Return the maximum number of bytes the heap could ever grow to.
 *
 * Equivalent to `_sstack - _sheap`: every byte between the heap base and
 * the bottom of the stack is theoretically reachable by sbrk. This number
 * is the *upper bound*, not what's actually safe to allocate.
 *
 * @return Bytes between heap base and stack floor.
 */
size_t get_max_heap_available(void);

/**
 * @brief Return the heap size guaranteed by the linker (_Min_Heap_Size).
 *
 * Equivalent to `_eheap - _sheap`. This is the amount the linker has
 * reserved purely for the heap; allocations beyond this size start eating
 * into the gap between heap and stack.
 *
 * @return Reserved heap size in bytes.
 */
size_t get_max_heap_reserved(void);

void hardfault_c(uint32_t* fault_sp);
