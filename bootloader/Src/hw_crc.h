#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialise the STM32F4 hardware CRC peripheral.
 *
 * Enables the CRC peripheral clock and resets its data register. Idempotent.
 */
void hw_crc_init(void);

/**
 * @brief Reset and power down the CRC peripheral before handing control to the application.
 *
 * Resets the CRC data register, disables the CRC peripheral clock and applies
 * a peripheral reset on AHB1 so the app sees a clean state.
 */
void hw_crc_deinit(void);

/**
 * @brief Compute a CRC-32/MPEG-2 over a byte buffer using the hardware CRC unit.
 *
 * Polynomial 0x04C11DB7, init 0xFFFFFFFF, no input/output reflection, no final
 * XOR — the only configuration the F4 CRC peripheral supports. Matches
 * `crcmod`'s predefined `crc-32-mpeg`.
 *
 * Tail bytes (when @p length is not a multiple of 4) are zero-padded to a full
 * 32-bit word; the host side must apply the same padding to match.
 *
 * @param data   Pointer to the input bytes.
 * @param length Number of bytes to hash.
 * @return CRC-32/MPEG-2 of the input.
 */
uint32_t hw_crc_calculate(const uint8_t* data, size_t length);
