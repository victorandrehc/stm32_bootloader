#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Send data over UART1.
 *
 * Transmits a buffer of data through the UART1 interface.
 *
 * @param buf Pointer to the data buffer to send.
 * @param len Number of bytes to transmit.
 *
 * @return int Status code (0 for success, negative for error).
 */
int uart1_send(const uint8_t* buf, size_t len);

/**
 * @brief Receive data over UART1.
 *
 * Receives data from the UART1 interface, blocking until the requested
 * number of bytes is received or the timeout expires.
 *
 * @param buf Pointer to the buffer to store received data.
 * @param len Number of bytes to receive.
 * @param timeout_ms Timeout duration in milliseconds.
 *
 * @return int Number of bytes received, or negative value on error.
 */
int uart1_recv(uint8_t* buf, size_t len, uint32_t timeout_ms);

/**
 * @brief Start UART1 interrupt-driven operation.
 *
 * Initializes and enables UART1 interrupts required for asynchronous
 * transmit and receive operations.
 *
 * @return int Status code (0 for success, negative for error).
 */
int uart_start_it(void);
