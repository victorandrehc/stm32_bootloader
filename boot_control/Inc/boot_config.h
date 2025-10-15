#pragma once

#include <stdint.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define FLASH_CFG __attribute__((section(".flash_cfg")))

#define BOOT_START_ADDR 0x8000000
#define APP_START_ADDR 0x08008000
#define BOOT_CONFIG_START_ADDR 0x0807FC00

typedef void (*jump_func_t)(void);

typedef struct PACKED bootloader_api_t{
    const jump_func_t jump_to_application;
    const jump_func_t jump_to_bootloader;
}bootloader_api_t;

extern  FLASH_CFG volatile bootloader_api_t bootloader_api;
