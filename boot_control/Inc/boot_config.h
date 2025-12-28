#pragma once

#include <stdint.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define BOOT_CONFIG __attribute__((section(".BOOT_CONFIG")))

#define FLASH_SECTOR_0_START_ADDR 0x08000000
#define FLASH_SECTOR_0_SIZE (16*1024)

#define FLASH_SECTOR_1_START_ADDR 0x08004000
#define FLASH_SECTOR_1_SIZE (16*1024)

#define FLASH_SECTOR_2_START_ADDR 0x08008000
#define FLASH_SECTOR_2_SIZE (16*1024)

#define FLASH_SECTOR_3_START_ADDR 0x0800C000
#define FLASH_SECTOR_3_SIZE (16*1024)

#define FLASH_SECTOR_4_START_ADDR 0x08010000
#define FLASH_SECTOR_4_SIZE (64*1024)

#define FLASH_SECTOR_5_START_ADDR 0x08020000
#define FLASH_SECTOR_5_SIZE (128*1024)

#define FLASH_SECTOR_6_START_ADDR 0x08040000
#define FLASH_SECTOR_6_SIZE (128*1024)

#define FLASH_SECTOR_7_START_ADDR 0x08060000
#define FLASH_SECTOR_7_SIZE (128*1024)


#define BOOT_START_ADDR FLASH_SECTOR_0_START_ADDR
#define APP_START_ADDR FLASH_SECTOR_2_START_ADDR
#define APP_SECTOR_SIZE FLASH_SECTOR_2_SIZE
#define BOOT_CONFIG_START_ADDR 0x20017c00


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
    jump_to_app_func_t jump_to_application;
    reset_funct_t reset;
    boot_info_t boot_info;
}bootloader_api_t;

extern volatile bootloader_api_t* bootloader_api_ptr;
void init_boot_api();

const char* get_reset_reason_string();

