#include "flash_handler.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "boot_config.h"
#include "main.h"

typedef struct{
    const uint32_t sector_id; // e.g.: FLASH_SECTOR_2
    const uint32_t start_addr;
    const size_t length_bytes;
    bool erased;
} flash_handler_t;

#define BYTES_TO_WORDS(SIZE) (SIZE/sizeof(uint32_t))
#define SECTOR_SIZE_BYTES_MAX FLASH_SECTOR_3_SIZE

#define FLASH_HANDLER_ARRAY_SIZE 6
flash_handler_t flash_handler_array[FLASH_HANDLER_ARRAY_SIZE]={
    {FLASH_SECTOR_2, FLASH_SECTOR_2_START_ADDR, FLASH_SECTOR_2_SIZE, false},
    {FLASH_SECTOR_3, FLASH_SECTOR_3_START_ADDR, FLASH_SECTOR_3_SIZE, false},
};
static size_t current_sector_pivot = 0;


static uint8_t sector[SECTOR_SIZE_BYTES_MAX];
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

static int flash_erase_once(flash_handler_t* current_sector)
{
    if (current_sector->erased)
    {
        return 0;
    }

    FLASH_EraseInitTypeDef erase;
    uint32_t sectorError;

    HAL_FLASH_Unlock();

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector       = current_sector->sector_id;
    erase.NbSectors    = 1;

    if (HAL_FLASHEx_Erase(&erase, &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return -1;
    }

    HAL_FLASH_Lock();
    current_sector->erased = true;
    return 0;
}

static void increment_current_sector()
{
    current_sector_pivot++;
    if(current_sector_pivot >= FLASH_HANDLER_ARRAY_SIZE)
    {
        printf("ALL SECTORS HAVE BEEN FLASHED\n");
        current_sector_pivot = 0;
    }
}

static int flash_write_sector(const uint8_t *buf)
{
   
    flash_handler_t* current_sector = &flash_handler_array[current_sector_pivot];
     printf("Writting sector ADDR: 0x%08lx SIZE: %u bytes\n", current_sector->start_addr, current_sector->length_bytes);
    if(current_sector->erased == false)
    {
       flash_erase_once(current_sector);
    }

    uint32_t address = current_sector->start_addr;
    const size_t length_words = BYTES_TO_WORDS(current_sector->length_bytes);
    const uint32_t *data = (const uint32_t *)buf;

    HAL_FLASH_Unlock();

    for (size_t i = 0; i < length_words; i++)
    {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                address,
                data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return -1;
        }
        address += 4;
    }

    HAL_FLASH_Lock();

    return 0;
}


static void flash_fw_feed_internal(const uint8_t *buf, size_t len)
{
    size_t offset = 0;

    while (offset < len) 
    {
        flash_handler_t* current_sector = &flash_handler_array[current_sector_pivot];
        size_t space = current_sector->length_bytes - pivot;
        size_t copy_len = (len - offset < space) ? (len - offset) : space;

        memcpy(&sector[pivot], &buf[offset], copy_len);
        pivot  += copy_len;
        offset += copy_len;

        if (pivot == current_sector->length_bytes) 
        {
            printf("SECTOR IS FULL\n");
            int ret = flash_write_sector(sector);
            if(ret == 0)
            {
                increment_current_sector();
            }
            flash_fw_reset();   // reset pivot + buffer
        }
    }
}


static void flash_fw_flush_internal(void)
{
    printf("FLUSHING SECTOR\n");
    if (pivot == 0)
    {
        return;
    }

    flash_handler_t* current_sector = &flash_handler_array[current_sector_pivot];
    // pad current sector with trash data
    memset(&sector[pivot], 0xFF, current_sector->length_bytes - pivot);
    //write sector
    int ret = flash_write_sector(sector);
    if(ret == 0)
    {
        increment_current_sector();
    }
    flash_fw_reset();  // reset pivot + buffer
}

void flash_fw_reset(void)
{
    // print_buffer(sector,SECTOR_SIZE_BYTES);
    pivot = 0;
    memset(sector, 0xFF, SECTOR_SIZE_BYTES_MAX);
}

int flash_fw_feed(const uint8_t *buf, size_t len)
{
    flash_fw_feed_internal(buf,len);
    return 0;
}

int flash_fw_flush(void)
{
    flash_fw_flush_internal();
    return 0;
}


