#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

/**
 * @def FIRMWARE_HEADER_MAGIC
 * @brief Magic number identifying a valid firmware header ("FWMH").
 */
#define FIRMWARE_HEADER_MAGIC 0x464D5748

/**
 * @def FIRMWARE_HEADER_VERSION
 * @brief Current version of the firmware header layout.
 */
#define FIRMWARE_HEADER_VERSION 1

/**
 * @brief Firmware header structure
 *
 * This header is placed at the beginning of the application firmware
 * to provide metadata and validation information.
 */
typedef struct PACKED firmware_header_t
{
    uint32_t magic;             ///< Magic number (FIRMWARE_HEADER_MAGIC)
    uint32_t header_version;    ///< Header structure version
    uint32_t firmware_version;  ///< Application firmware version
    uint32_t firmware_size;     ///< Size of firmware in bytes (excluding header)
    uint32_t firmware_crc;      ///< CRC32 of firmware (excluding header)
    uint32_t build_timestamp;   ///< Unix timestamp of build
    uint32_t reserved[2];       ///< Reserved for future use
} firmware_header_t;

/**
 * @def FIRMWARE_HEADER_SIZE
 * @brief Size in bytes of the firmware header.
 */
#define FIRMWARE_HEADER_SIZE sizeof(firmware_header_t)

/**
 * @brief Validate firmware header
 *
 * @param header Pointer to firmware header
 * @return true if header is valid, false otherwise
 */
bool firmware_header_validate(const firmware_header_t* header);

/**
 * @brief Calculate CRC32 for firmware validation
 *
 * @param data Pointer to data
 * @param length Length of data in bytes
 * @return CRC32 value
 */
uint32_t firmware_calculate_crc32(const uint8_t* data, uint32_t length);

/**
 * @brief Verify firmware integrity using header information
 *
 * @param app_address Starting address of application (with header)
 * @return true if firmware is valid, false otherwise
 */
bool firmware_verify_integrity(uint32_t app_address);

/**
 * @brief Get firmware header from application address
 *
 * @param app_address Starting address of application (with header)
 * @return Pointer to firmware header
 */
const firmware_header_t* firmware_get_header(uint32_t app_address);
