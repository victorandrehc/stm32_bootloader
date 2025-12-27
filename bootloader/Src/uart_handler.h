#pragma once

#include <stdint.h>
#include <stddef.h>




int uart1_send(const uint8_t* buf, size_t len);

int uart1_recv(uint8_t *buf, size_t len, uint32_t timeout_ms);

int uart_start_it();