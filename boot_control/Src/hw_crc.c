#include "hw_crc.h"

#include "stm32f4xx_hal.h"

#include <string.h>

void hw_crc_init(void)
{
    __HAL_RCC_CRC_CLK_ENABLE();
    CRC->CR = CRC_CR_RESET;
}

void hw_crc_deinit(void)
{
    CRC->CR = CRC_CR_RESET;
    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();
    __HAL_RCC_CRC_CLK_DISABLE();
}

uint32_t hw_crc_calculate(const uint8_t* data, size_t length)
{
    /* Reset DR back to 0xFFFFFFFF before each computation. */
    CRC->CR = CRC_CR_RESET;

    const size_t full_words = length / sizeof(uint32_t);
    const size_t tail_bytes = length % sizeof(uint32_t);

    /* The CRC unit consumes each 32-bit word MSB-first. Firmware lives in
     * memory little-endian, so byte-swap each word to feed bytes in the same
     * order a byte-stream CRC-32/MPEG-2 would. */
    for (size_t i = 0; i < full_words; i++)
    {
        uint32_t word;
        memcpy(&word, data + i * sizeof(uint32_t), sizeof(word));
        CRC->DR = __REV(word);
    }

    /* Tail (1-3 leftover bytes): the unit only takes 32-bit writes, so pack
     * leftovers MSB-first into one word, leave low bytes at 0x00. The unit
     * then processes "data + zero padding". Host pads the same way.
     * Example: 0xAA 0xBB -> last = 0xAABB0000 -> consumed as AA,BB,00,00. */
    if (tail_bytes != 0)
    {
        uint32_t last = 0;
        for (size_t i = 0; i < tail_bytes; i++)
        {
            last |= ((uint32_t) data[full_words * sizeof(uint32_t) + i]) << (24 - i * 8);
        }
        CRC->DR = last;
    }

    return CRC->DR;
}
