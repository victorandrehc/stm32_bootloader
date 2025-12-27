#pragma once

#include <stdint.h>
#include <stddef.h>


typedef  int (*uart_send)(const uint8_t *buf, size_t len);
typedef  int (*uart_recv)(uint8_t *buf, size_t len, uint32_t timeout_ms);

typedef struct {
   uart_send send;
   uart_recv recv;
} serial_api_t;

void set_serial_api(uart_send send, uart_recv recv);

int recv_firmware();