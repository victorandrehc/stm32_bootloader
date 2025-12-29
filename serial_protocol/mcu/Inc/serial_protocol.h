#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*uart_send_t)(const uint8_t* buf, size_t len);
typedef int (*uart_recv_t)(uint8_t* buf, size_t len, uint32_t timeout_ms);
typedef int (*flash_feed_t)(const uint8_t* buf, size_t len);
typedef int (*flash_flush_t)(void);
typedef void (*flash_reset_t)(void);
typedef bool (*fw_crc_check_t)(uint16_t crc_recv, size_t fw_len);
typedef int (*fw_write_header_t)(uint16_t crc_recv, size_t fw_len);

typedef struct
{
    uart_send_t send;
    uart_recv_t recv;
    flash_feed_t flash_feed;
    flash_flush_t flash_flush;
    flash_reset_t flash_reset;
    fw_crc_check_t fw_crc_check;
    fw_write_header_t fw_write_header;
    size_t max_fw_size;
} serial_api_t;

void set_serial_api(serial_api_t api);

int recv_firmware();