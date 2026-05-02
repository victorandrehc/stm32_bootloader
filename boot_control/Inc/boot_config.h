#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
/**
 * @def PACKED
 * @brief Attribute to pack structures without padding.
 */
#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

/**
 * @def BOOT_CONFIG
 * @brief Attribute to place data into the boot configuration linker section. A noinit RAM section
 */
#define BOOT_CONFIG __attribute__((section(".BOOT_CONFIG")))

/* -------------------------------------------------------------------------- */
/* Flash memory layout                                                         */
/* -------------------------------------------------------------------------- */

/**
 * @name Flash sector start addresses and sizes
 * @{
 */
#define FLASH_SECTOR_0_START_ADDR 0x08000000
#define FLASH_SECTOR_0_SIZE       (16 * 1024)

#define FLASH_SECTOR_1_START_ADDR 0x08004000
#define FLASH_SECTOR_1_SIZE       (16 * 1024)

#define FLASH_SECTOR_2_START_ADDR 0x08008000
#define FLASH_SECTOR_2_SIZE       (16 * 1024)

#define FLASH_SECTOR_3_START_ADDR 0x0800C000
#define FLASH_SECTOR_3_SIZE       (16 * 1024)

#define FLASH_SECTOR_4_START_ADDR 0x08010000
#define FLASH_SECTOR_4_SIZE       (64 * 1024)

#define FLASH_SECTOR_5_START_ADDR 0x08020000
#define FLASH_SECTOR_5_SIZE       (128 * 1024)

#define FLASH_SECTOR_6_START_ADDR 0x08040000
#define FLASH_SECTOR_6_SIZE       (128 * 1024)

#define FLASH_SECTOR_7_START_ADDR 0x08060000
#define FLASH_SECTOR_7_SIZE       (128 * 1024)
/** @} */

/* -------------------------------------------------------------------------- */
/* Firmware layout                                                             */
/* -------------------------------------------------------------------------- */

/**
 * @def FW_HEADER_SIZE
 * @brief Size of the firmware header in bytes.
 */
#define FW_HEADER_SIZE 0x200

/**
 * @def RAM_CFG_SIZE
 * @brief Size of the configuration ram section header in bytes. THis is the no-init ram
 */
#define RAM_CFG_SIZE 0x400

/**
BOOT_CONFIG is the only symbol in RAM_CFG;
*/
#define BOOT_CFG_SIZE RAM_CFG_SIZE

/**
 * @def BOOT_START_ADDR
 * @brief Start address of the bootloader.
 */
#define BOOT_START_ADDR FLASH_SECTOR_0_START_ADDR

/**
 * @def APP_START_ADDR
 * @brief Start address of the application firmware.
 */
#define APP_START_ADDR (FLASH_SECTOR_2_START_ADDR + FW_HEADER_SIZE)

/**
 * @def APP_SECTOR_SIZE
 * @brief Size of the application flash sector.
 */
#define APP_SECTOR_SIZE FLASH_SECTOR_2_SIZE

/**
 * @def BOOT_CONFIG_START_ADDR
 * @brief Start address of boot configuration storage in RAM or flash.
 */
#define BOOT_CONFIG_START_ADDR 0x20017c00

/**
 * @brief Firmware header stored at the beginning of the application image.
 *
 * This header contains metadata used by the bootloader to validate and
 * load the application firmware.
 */
typedef struct __attribute__((packed, aligned(4)))
{
    uint32_t magic;   /**< Firmware magic number */
    uint32_t fw_size; /**< Firmware size in bytes (excluding header) */
    uint32_t crc;     /**< CRC of the firmware image only */
    uint8_t reserved[FW_HEADER_SIZE - 3 * sizeof(uint32_t)];
    /**< Reserved for future use and padding */
} fw_header_t;

/**
 * @brief Compile-time check to ensure firmware header size correctness.
 */
_Static_assert(sizeof(fw_header_t) == FW_HEADER_SIZE, "FIRMWARE HEADER SIZE MISMATCH");

/* -------------------------------------------------------------------------- */
/* Reset handling                                                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Reset reason enumeration.
 */
typedef enum reset_reason_e
{
    POWER_CYCLE,       /**< Device powered up from cold start */
    APPLICATION_RESET, /**< Reset requested by application */
    FIRMWARE_UPDATE,   /**< Reset triggered after firmware update */
    HARD_FAULT,        /**< Reset triggered after Hard Fault beig called */
    UNKNOWN,
} reset_reason_e;

/**
 * @def BOOT_INFO_MAGIC
 * @brief Magic value used to validate boot information.
 */
#define BOOT_INFO_MAGIC 0xDEADBEEFU

/**
 * @def CRASH_DUMP_STACK_WORDS
 * @brief Number of 32-bit stack words captured into the crash dump.
 *
 * Sized so the full crash_dump_t fits inside the BOOT_CONFIG noinit RAM region.
 */
#define CRASH_DUMP_STACK_WORDS 237

/**
 * @def CRASH_DUMP_STACK_BYTES
 * @brief Size in bytes of the captured stack snapshot in a crash dump.
 */
#define CRASH_DUMP_STACK_BYTES (CRASH_DUMP_STACK_WORDS * sizeof(uint32_t))

/**
 * @brief Persisted snapshot of CPU state captured on a HardFault.
 *
 * Populated by the fault handler before reset and read by the bootloader on
 * the next boot to report the failure. Lives in the BOOT_CONFIG noinit RAM
 * section so it survives a warm reset.
 */
typedef struct PACKED crash_dump_t
{
    uint32_t sp_at_fault;                   /**< Stack pointer (MSP/PSP) at the moment of fault entry */
    uint32_t cfsr;                          /**< Configurable Fault Status Register */
    uint32_t hfsr;                          /**< HardFault Status Register */
    uint32_t mmfar;                         /**< MemManage Fault Address Register */
    uint32_t bfar;                          /**< BusFault Address Register */
    uint32_t hw_frame[8];                   /**< Hardware-stacked exception frame: R0-R3, R12, LR, PC, xPSR */
    size_t stack_captured;                  /**< Number of valid words in @ref stack (<= CRASH_DUMP_STACK_WORDS) */
    uint32_t stack[CRASH_DUMP_STACK_WORDS]; /**< Stack words copied starting at @ref sp_at_fault */
} crash_dump_t;

/**
 * @brief Boot information structure shared between bootloader and application.
 */
typedef struct PACKED boot_info_t
{
    uint32_t magic;             /**< Validation magic value */
    uint32_t reset_reason_uint; /**< Reset reason as integer */
    crash_dump_t crash_dump;    /**< Last-fault snapshot, valid when reset_reason == HARD_FAULT */
} boot_info_t;
_Static_assert(sizeof(boot_info_t) <= BOOT_CFG_SIZE, "BOOT_CONFIG_MISMATCH");

/* -------------------------------------------------------------------------- */
/* Bootloader API                                                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Function pointer type to jump to the application.
 */
typedef void (*jump_to_app_func_t)(void);

/**
 * @brief Function pointer type to reset the system with a reason.
 *
 * @param reset_reason Reason for triggering the reset.
 */
typedef void (*reset_funct_t)(const reset_reason_e reset_reason);

/**
 * @brief Bootloader API exposed to the application.
 *
 * This structure provides controlled entry points for the application
 * to interact with the bootloader.
 */
typedef struct PACKED bootloader_api_t
{
    jump_to_app_func_t jump_to_application; /**< Jump to application entry */
    reset_funct_t reset;                    /**< Trigger system reset */
    boot_info_t boot_info;                  /**< Persistent boot information */
} bootloader_api_t;

/**
 * @brief Pointer to the bootloader API structure.
 *
 * This pointer is expected to be initialized by the bootloader and
 * accessed by the application.
 *
 * This Poiinter should point to an address inside the BOOT_CONFIG noinit RAM section
 */
extern volatile bootloader_api_t* bootloader_api_ptr;

/**
 * @brief Initialize the bootloader API.
 *
 * Sets up the bootloader API structure and makes it available to
 * the application.
 */
void init_boot_api(void);

/**
 * @brief Get the reset reason recorded in the boot info structure.
 *
 * Returns the reason that triggered the most recent reset. Valid only after
 * the bootloader has populated the boot info on startup.
 *
 * @return reset_reason_e Last recorded reset reason.
 */
reset_reason_e get_reset_reason(void);

/**
 * @brief Clear the recorded reset reason in the boot info structure.
 *
 * Resets the stored reason back to its default. Typically called by the
 * application once it has consumed the value returned by
 * @ref get_reset_reason.
 */
void clear_reset_reason(void);

/**
 * @brief Get a human-readable string describing the last reset reason.
 *
 * @return const char* Null-terminated string describing the reset reason.
 */
const char* get_reset_reason_string(void);
