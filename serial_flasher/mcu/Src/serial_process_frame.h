#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Serial protocol command identifiers.
 *
 * These values represent the supported commands exchanged between
 * the host and the device during the serial firmware flashing process.
 */
typedef enum
{
    CMD_UNKNOWN = 0,  /**< Unknown or invalid command */
    CMD_PING = 0x01,  /**< Ping command to start communication */
    CMD_START = 0x02, /**< Start of firmware transmission */
    CMD_DATA = 0x03,  /**< Firmware data packet */
    CMD_END = 0x04,   /**< End of firmware transmission */
    CMD_RESET = 0x05, /**< Reset command after flashing */

    CMD_ACK = 0x7F,  /**< Acknowledge command */
    CMD_NACK = 0x7E, /**< Negative acknowledge command */
} serial_cmd_t;

/**
 * @brief Receive a framed serial command.
 *
 * This function receives a frame from the serial interface, decodes
 * the command, and provides access to the associated payload.
 *
 * @param[out] cmd Pointer to store the received command.
 * @param[out] payload Pointer to the received payload buffer.
 *                     The buffer is managed internally and must not be freed.
 * @param[out] len Pointer to store the payload length in bytes.
 *
 * @return true If a valid frame was successfully received.
 * @return false If the frame was invalid or reception failed.
 */
bool recv_frame(serial_cmd_t* cmd, uint8_t** payload, size_t* len);

/**
 * @brief Send an acknowledge (ACK) response.
 *
 * Sends an ACK frame to indicate successful reception and processing
 * of the last command.
 */
void send_ack(void);

/**
 * @brief Send a negative acknowledge (NACK) response.
 *
 * Sends a NACK frame to indicate an error in reception or processing
 * of the last command.
 */
void send_nack(void);
