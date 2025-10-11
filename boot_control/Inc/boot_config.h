#pragma once

#include <stdint.h>

#define BOOT_START_ADDR 0x8000000
#define APP_START_ADDR 0x08008000



void jump_to_application(uintptr_t app_addr);