#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Feed firmware data into flash memory.
 *
 * Buffers a chunk of firmware data to be written in flash. This function is typically
 * called repeatedly as firmware data packets are received. The data will be written when the sector is full or when flash_fw_flush is
 * called
 *
 * @param buf Pointer to the firmware data buffer.
 * @param len Number of bytes to write.
 *
 * @return int Status code (0 for success, negative for error).
 */
int flash_fw_feed(const uint8_t* buf, size_t len);

/**
 * @brief Flush pending firmware data to flash memory.
 *
 * Writes the current buffered data in flash, it pads the remaining address of the sector with 0xFF
 *
 * @return int Status code (0 for success, negative for error).
 */
int flash_fw_flush(void);

/**
 * @brief Reset the firmware flashing state.
 *
 * Resets any internal state related to firmware programming and
 * prepares the system for a new firmware update or normal operation.
 */
void flash_fw_reset(void);

/**
 * @brief Get the maximum supported firmware size.
 *
 * @return size_t Maximum firmware size in bytes.
 */
size_t get_max_fw_size(void);

/**
 * @brief Verify the CRC of the received firmware.
 *
 * Compares the received CRC value against a calculated CRC over
 * the programmed firmware data.
 *
 * @param crc_recv CRC value received from the host.
 * @param fw_len Length of the firmware in bytes.
 *
 * @return true If the CRC matches.
 * @return false If the CRC check fails.
 */
bool fw_crc_check(uint16_t crc_recv, size_t fw_len);

/**
 * @brief Buffers the firmware header to flash.
 *
 * Stores metadata such as firmware length and CRC into a
 * header region for validation during boot.
 *
 * The data will be written when the sector is full or when flash_fw_flush is called
 *
 * @param crc_recv CRC value of the firmware.
 * @param fw_len Length of the firmware in bytes.
 *
 * @return int Status code (0 for success, negative for error).
 */
int fw_write_header(uint16_t crc_recv, size_t fw_len);

/**
 * @brief Validate the firmware header stored in flash.
 *
 * Checks the integrity and correctness of the firmware header,
 * typically during system startup or before firmware execution.
 *
 * @return int Status code (0 if the header is valid, negative for error).
 */
int fw_check_header(void);
