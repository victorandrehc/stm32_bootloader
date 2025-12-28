#include "flash_handler.h"
#include <string.h>
#include <stdio.h>


typedef enum {
    FLASH_OK = 0,
    FLASH_SECTOR_READY,
    FLASH_ERROR
} flash_status_t;

#define SECTOR_SIZE (4 * 1024)

static uint8_t sector[SECTOR_SIZE];
static size_t pivot = 0;

static void print_buffer(const uint8_t *buf, size_t len)
{
    printf("SECTOR [%u bytes]: ", len);
    for (size_t i = 0; i < len; i++) {
        printf("%c[%x] ", buf[i], buf[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

static flash_status_t flash_fw_feed_internal(const uint8_t *buf, size_t len)
{
    size_t offset = 0;

    while (offset < len) {
        size_t space = SECTOR_SIZE - pivot;
        size_t copy_len = (len - offset < space) ? (len - offset) : space;

        memcpy(&sector[pivot], &buf[offset], copy_len);
        pivot += copy_len;
        offset += copy_len;

        if (pivot == SECTOR_SIZE) {
            // Sector is full

            // FUTURE: trigger flash write here
            // flash_write_sector(sector);
            flash_fw_reset();
            return FLASH_SECTOR_READY;
        }
    }

    return FLASH_OK;
}

static flash_status_t flash_fw_flush_internal(void)
{
    if (pivot == 0)
    {
        return FLASH_OK;
    }

    // FUTURE: flash write partial sector if allowed
    // flash_write_partial(sector, pivot);

    pivot = 0;
    return FLASH_SECTOR_READY;
}

void flash_fw_reset(void)
{
    pivot = 0;
    memset(sector, 0xFF, SECTOR_SIZE);
}

int flash_fw_feed(const uint8_t *buf, size_t len)
{
    const flash_status_t ret = flash_fw_feed_internal(buf,len);
    return (ret != FLASH_ERROR? 0:-1);
}

int flash_fw_flush(void)
{
    const flash_status_t ret = flash_fw_flush_internal();
    return (ret != FLASH_ERROR? 0:-1);
}


