#include "firmware_header.h"

#include <stdint.h>

// Firmware version - update this with each release
#define APP_FIRMWARE_VERSION 1

// Build timestamp - this should ideally be set during the build process
// For now, we use a placeholder that can be updated
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP 0
#endif

// External symbols from linker script
extern uint32_t _etext;  // End of code section
extern uint32_t _firmware_header_start;

// Firmware header placed at the beginning of the application
// This header will be placed in a special section that comes before the vector table
__attribute__((section(".firmware_header"))) const firmware_header_t application_firmware_header = {
    .magic = FIRMWARE_HEADER_MAGIC,
    .header_version = FIRMWARE_HEADER_VERSION,
    .firmware_version = APP_FIRMWARE_VERSION,
    .firmware_size = 0, // This will be calculated and patched after build
    .firmware_crc = 0, // This will be calculated and patched after build
    .build_timestamp = BUILD_TIMESTAMP,
    .reserved = {0, 0}
};
