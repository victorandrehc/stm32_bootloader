# Crash dump: build fix + handler cleanup + capture body

## Context

On the `feature/stack_trace` branch, commit `9c6ad90` ("WIP") wires up
the start of a crash dump that survives reset by living in the
`.BOOT_CONFIG` noinit section. The build currently fails with a
linker error along the lines of `Unknown destination type ...
hardfault_c`. This plan diagnoses that error and outlines the
follow-up work needed to actually populate a useful dump.

## Why the build fails

`hardfault_c` is defined in `boot_control/Src/boot_config.c:103`, but
`boot_control/Src/boot_config.c` is **not in the app target's source
list** in `app/CMakeLists.txt:1-11`. Only `stack_config.c` from
`boot_control/Src/` is compiled.

The call chain in the app binary is:

```
HardFault_Handler  (naked, in app/Src/stm32f4xx_it.c)
  └─ bl hardfault_c_internal      (also in stm32f4xx_it.c)
       └─ hardfault_c             (defined in boot_config.c — not built)
```

Because `boot_config.c` is excluded from the app build, `hardfault_c`
has no definition the linker can see. With `-flto` enabled (see
`app/CMakeLists.txt:69`), the linker reports the unresolved reference
that comes through inline asm as `Unknown destination type ...`
instead of the more typical `undefined reference to ...`. Same root
cause, just a less helpful diagnostic.

## Plan

### Step 1 — fix the link error

Add `../boot_control/Src/boot_config.c` to the app's source list in
`app/CMakeLists.txt:1-11`:

```cmake
set(SOURCE_FILES
        ../Inc/main.h
        ../Inc/stm32f4xx_hal_conf.h
        ../Inc/stm32f4xx_it.h
        Src/main.c
        Src/stm32f4xx_hal_msp.c
        Src/stm32f4xx_it.c
        Src/system_stm32f4xx.c
        Src/startup_stm32f401xe.s
        Src/syscalls.c
        ../boot_control/Src/stack_config.c
        ../boot_control/Src/boot_config.c)        # NEW
```

After this the build should complete. Verify with:

```
ninja -C build cmake_stm32_app.out
```

Note: the existing `Wl,--no-warn-rwx-segment` link flag in
`app/CMakeLists.txt:70` requires binutils 2.36+. The current toolchain
in the working tree (gcc 10.3.1) doesn't support it. If this becomes a
blocker, either upgrade the toolchain or drop the flag — it's only a
warning suppression and unrelated to the crash dump work.

### Step 2 — simplify the HardFault handler

In `app/Src/stm32f4xx_it.c` (current commit lines 39–66 in the diff),
replace the trampoline + naked function with a single naked handler
that tail-branches directly to `hardfault_c`:

```c
__attribute__((naked, used)) void HardFault_Handler(void)
{
    __asm volatile (
        "tst   lr, #4           \n"   /* EXC_RETURN bit 2: 0 = MSP, 1 = PSP */
        "ite   eq               \n"
        "mrseq r0, msp          \n"
        "mrsne r0, psp          \n"
        "b     hardfault_c      \n"   /* tail-branch; hardfault_c never returns */
    );
}
```

Reasoning:

- Drop `hardfault_c_internal` — it's a pure pass-through.
- Drop the `extern void hardfault_c(...)` re-declaration — the
  prototype already lives in `boot_config.h:222`.
- Remove the `while (1) { }` C body. `naked` functions must not
  contain C statements; with `-Wall` the current code emits
  `warning: naked function ... contains code`. The reset call inside
  `hardfault_c` plus a final spin loop in C also handles non-return.
- Use `b` instead of `bl`. The C handler resets the chip, never
  returns, so saving LR is wasted.

### Step 3 — write a real capture body

Replace the placeholder body of `hardfault_c` in `boot_config.c`
(currently `printf("aqui\n"); reset(HARD_FAULT);`) with one that
populates the `crash_dump_t` already declared in `boot_config.h`.

Required externs from the linker (already provided by
`app/stm32f401_app.ld`):

```c
extern uint32_t _estack;
```

Capture body:

```c
#include "stm32f4xx_hal.h"   /* SCB definitions */
#include <string.h>

void hardfault_c(uint32_t *fault_sp)
{
    crash_dump_t *cd = &bootloader_api_ptr->boot_info.crash_dump;

    cd->sp_at_fault = (uint32_t)fault_sp;
    cd->cfsr        = SCB->CFSR;
    cd->hfsr        = SCB->HFSR;
    cd->mmfar       = SCB->MMFAR;
    cd->bfar        = SCB->BFAR;

    /* Hardware-stacked exception frame: R0-R3, R12, LR, PC, xPSR */
    for (size_t i = 0; i < 8; i++) {
        cd->hw_frame[i] = fault_sp[i];
    }

    /* Capture from current SP up toward _estack, clipped to buffer */
    const uint8_t *top = (const uint8_t *)&_estack;
    size_t available   = (size_t)(top - (const uint8_t *)fault_sp);
    if (available > CRASH_DUMP_STACK_BYTES) available = CRASH_DUMP_STACK_BYTES;

    cd->stack_captured           = available;
    cd->stack_captured_init_addr = (uint32_t)fault_sp;
    memcpy((void *)cd->stack, fault_sp, available);

    bootloader_api_ptr->reset(HARD_FAULT);
    while (1) { }   /* unreachable; keeps compiler happy */
}
```

Notes:

- `boot_config.c` already includes `stack_config.h` after the WIP
  commit, but it does **not** need it for this body. It does need
  `<string.h>` for `memcpy` and `stm32f4xx_hal.h` for `SCB`.
- The `bootloader_api_ptr` global is the existing pointer into the
  noinit region; using it (rather than touching `boot_info.crash_dump`
  by direct address) keeps a single source of truth for where the
  dump lives.
- The `printf("aqui\n")` should be removed. `printf` from a fault
  context is unsafe — UART may already be in a bad state, and the
  vfprintf scratch buffer would chew through the very stack we want
  to capture.

### Step 4 — read the dump on next boot

Add to `boot_control/Src/boot_config.c` a small print routine, called
from the bootloader's main once near boot:

```c
void crash_dump_print(void)
{
    if (bootloader_api_ptr->boot_info.reset_reason_uint != HARD_FAULT) {
        return;
    }
    const crash_dump_t *cd = &bootloader_api_ptr->boot_info.crash_dump;

    printf("=== CRASH DUMP ===\n");
    printf("sp_at_fault: 0x%08lx\n", (unsigned long)cd->sp_at_fault);
    printf("CFSR=0x%08lx HFSR=0x%08lx MMFAR=0x%08lx BFAR=0x%08lx\n",
           (unsigned long)cd->cfsr,  (unsigned long)cd->hfsr,
           (unsigned long)cd->mmfar, (unsigned long)cd->bfar);
    printf("hw frame: r0=%08lx r1=%08lx r2=%08lx r3=%08lx\n",
           (unsigned long)cd->hw_frame[0], (unsigned long)cd->hw_frame[1],
           (unsigned long)cd->hw_frame[2], (unsigned long)cd->hw_frame[3]);
    printf("          r12=%08lx lr=%08lx pc=%08lx xpsr=%08lx\n",
           (unsigned long)cd->hw_frame[4], (unsigned long)cd->hw_frame[5],
           (unsigned long)cd->hw_frame[6], (unsigned long)cd->hw_frame[7]);
    printf("stack: %u bytes from 0x%08lx\n",
           (unsigned)cd->stack_captured,
           (unsigned long)cd->stack_captured_init_addr);

    const size_t n = cd->stack_captured / sizeof(uint32_t);
    for (size_t i = 0; i < n; i++) {
        printf("  [%02u] 0x%08lx: 0x%08lx\n",
               (unsigned)i,
               (unsigned long)(cd->stack_captured_init_addr + i * 4u),
               (unsigned long)cd->stack[i]);
    }
}
```

Add a prototype in `boot_config.h` and wire a single call to
`crash_dump_print()` early in the bootloader's `main` (or app's `main`,
depending on which UART is convenient). The dump only prints when the
last reset reason was `HARD_FAULT`, so it's a no-op on cold boots.

Optionally: clear `boot_info.reset_reason_uint = POWER_CYCLE` after
printing so subsequent resets don't keep replaying the same crash.

## Files modified

- `app/CMakeLists.txt` — add `boot_config.c` to source list.
- `app/Src/stm32f4xx_it.c` — simplify `HardFault_Handler`, remove the
  `_internal` trampoline.
- `boot_control/Src/boot_config.c` — replace placeholder
  `hardfault_c` body with the real capture; add `crash_dump_print`.
- `boot_control/Inc/boot_config.h` — add `crash_dump_print()` prototype;
  add trailing newline.

## Verification

1. Clean build:
   ```
   rm -rf build && cmake -B build -G Ninja && cmake --build build
   ```
   Expect no `Unknown destination type` / undefined reference and no
   `naked function ... contains code` warning.

2. Flash the app, hit the existing `__asm volatile ("udf #0")` in
   `app/Src/main.c:64` to deliberately trigger a HardFault.

3. After reset, the bootloader should print the crash dump line,
   showing a non-zero `cfsr` (UNDEFINSTR bit set), a `pc` pointing at
   the `udf` instruction in flash, and a stack listing whose top
   words match the live frames at fault time.

4. Confirm the dump magic / reset_reason logic correctly suppresses
   printing on a cold power-on (no spurious dump).
