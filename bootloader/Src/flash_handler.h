#pragma once
#include <stddef.h>
#include <stdint.h>


int flash_fw_feed(const uint8_t *buf, size_t len);
int flash_fw_flush(void);
void flash_fw_reset(void);
