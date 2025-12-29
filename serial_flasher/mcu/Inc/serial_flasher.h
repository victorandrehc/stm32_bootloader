#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Function pointer type for sending data over UART.
 *
 * @param buf Pointer to the buffer containing data to send.
 * @param len Length of the data to send in bytes.
 * @return int Status code (0 for success, negative for error).
 */
typedef int (*uart_send_t)(const uint8_t* buf, size_t len);

/**
 * @brief Function pointer type for receiving data over UART.
 *
 * @param buf Pointer to the buffer to store received data.
 * @param len Number of bytes to receive.
 * @param timeout_ms Timeout in milliseconds for receiving data.
 * @return int Number of bytes actually received, or negative for error.
 */
typedef int (*uart_recv_t)(uint8_t* buf, size_t len, uint32_t timeout_ms);

/**
 * @brief Function pointer type for feeding data to flash memory.
 *
 * @param buf Pointer to the buffer containing data to write to flash.
 * @param len Number of bytes to write.
 * @return int Status code (0 for success, negative for error).
 */
typedef int (*flash_feed_t)(const uint8_t* buf, size_t len);

/**
 * @brief Function pointer type for flushing flash memory writes.
 *
 * @return int Status code (0 for success, negative for error).
 */
typedef int (*flash_flush_t)(void);

/**
 * @brief Function pointer type for resetting the flash memory handling state machine.
 * This Does not erase the flash or overwrite already written sectors, but allows already written sectors to be rewritten
 */
typedef void (*flash_reset_t)(void);

/**
 * @brief Function pointer type for checking firmware CRC.
 *
 * @param crc_recv CRC value received for comparison.
 * @param fw_len Length of the firmware in bytes.
 * @return true If CRC matches, false otherwise.
 */
typedef bool (*fw_crc_check_t)(uint16_t crc_recv, size_t fw_len);

/**
 * @brief Function pointer type for writing the firmware header.
 *
 * @param crc_recv CRC value of the firmware.
 * @param fw_len Length of the firmware in bytes.
 * @return int Status code (0 for success, negative for error).
 */
typedef int (*fw_write_header_t)(uint16_t crc_recv, size_t fw_len);

/**
 * @brief API structure used by the serial flasher state machine.
 *
 * This structure contains all necessary function pointers and parameters
 * to handle UART communication and flash memory operations for firmware
 * updating.
 */
typedef struct
{
    uart_send_t send;                  /**< Function to send data over UART */
    uart_recv_t recv;                  /**< Function to receive data over UART */
    flash_feed_t flash_feed;           /**< Function to feed data to flash memory */
    flash_flush_t flash_flush;         /**< Function to flush flash memory writes */
    flash_reset_t flash_reset;         /**< Function to reset flash memory or device */
    fw_crc_check_t fw_crc_check;       /**< Function to check firmware CRC */
    fw_write_header_t fw_write_header; /**< Function to write firmware header */
    size_t max_fw_size;                /**< Maximum firmware size supported */
} serial_api_t;

/**
 * @brief Set the serial API for the flasher state machine.
 *
 * @param api Structure containing all required UART and flash functions.
 */
void set_serial_api(serial_api_t api);

/**
 * @brief Run the firmware receiving state machine.
 *
 * This function handles the reception of firmware over UART and
 * programming it into flash memory using the provided API.
 *
 * @return int Status code (0 for success, negative for error).
 */
int recv_firmware(void);
