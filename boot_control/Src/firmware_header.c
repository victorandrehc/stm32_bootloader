#include "firmware_header.h"

#include "boot_config.h"
#include "hw_crc.h"

#include <string.h>

uint32_t firmware_calculate_crc32(const uint8_t* data, uint32_t length)
{
    return hw_crc_calculate(data, length);
}

bool firmware_header_validate(const firmware_header_t* header)
{
    if (header == NULL)
    {
        return false;
    }

    // Check magic number
    if (header->magic != FIRMWARE_HEADER_MAGIC)
    {
        return false;
    }

    // Check header version
    if (header->header_version != FIRMWARE_HEADER_VERSION)
    {
        return false;
    }

    // Check firmware size is reasonable (not zero, not too large)
    if (header->firmware_size == 0 || header->firmware_size > (480 * 1024))
    {
        return false;
    }

    return true;
}

const firmware_header_t* firmware_get_header(uint32_t app_address)
{
    return (const firmware_header_t*) app_address;
}

bool firmware_verify_integrity(uint32_t app_address)
{
    const firmware_header_t* header = firmware_get_header(app_address);

    // Validate header structure
    if (!firmware_header_validate(header))
    {
        return false;
    }

    // Calculate CRC of the firmware (skip the header)
    const uint8_t* firmware_data = (const uint8_t*) (app_address + FIRMWARE_HEADER_SIZE);
    uint32_t calculated_crc = firmware_calculate_crc32(firmware_data, header->firmware_size);

    // Compare with stored CRC
    if (calculated_crc != header->firmware_crc)
    {
        return false;
    }

    return true;
}
