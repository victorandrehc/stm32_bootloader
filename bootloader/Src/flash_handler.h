#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int flash_fw_feed(const uint8_t* buf, size_t len);
int flash_fw_flush(void);
void flash_fw_reset(void);
size_t get_max_fw_size();
bool fw_crc_check(uint16_t crc_recv, size_t fw_len);
int fw_write_header(uint16_t crc_recv, size_t fw_len);
int fw_check_header(void);
