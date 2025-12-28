#pragma once

#include <stdint.h>
#include <stddef.h>


typedef  int (*uart_send_t)(const uint8_t *buf, size_t len);
typedef  int (*uart_recv_t)(uint8_t *buf, size_t len, uint32_t timeout_ms);
typedef  int (*flash_feed_t)(const uint8_t *buf, size_t len);
typedef  int (*flash_flush_t)(void);
typedef  void (*flash_reset_t)(void);


typedef struct {
   uart_send_t send;
   uart_recv_t recv;
   flash_feed_t flash_feed; 
   flash_flush_t flash_flush;
   flash_reset_t flash_reset;
} serial_api_t;

void set_serial_api(uart_send_t send, uart_recv_t recv, flash_feed_t flash_feed,
    flash_flush_t flash_flush, flash_reset_t flash_reset);

int recv_firmware();