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


typedef enum reset_reason_e{
 POWER_CYCLE,
 APPLICATION_RESET,
 FIRMWARE_UPDATE,
}reset_reason_e;

#define BOOT_INFO_MAGIC 0xDEADBEEFU

typedef struct PACKED boot_info_t{
    uint32_t magic;
    uint32_t reset_reason_uint;
    uint32_t fw_version;
    uint32_t reserved[5];    
}boot_info_t;

typedef void (*jump_to_app_func_t)(void);
typedef void (*reset_funct_t)(const reset_reason_e reset_reason);

typedef struct PACKED bootloader_api_t{
    const jump_to_app_func_t jump_to_application;
    const reset_funct_t reset;
    boot_info_t boot_info;
}bootloader_api_t;

extern  FLASH_CFG volatile bootloader_api_t bootloader_api;
const char* get_reset_reason_string();
