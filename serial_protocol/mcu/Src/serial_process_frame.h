#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    CMD_UNKNOWN = 0,
    CMD_PING = 0x01,
    CMD_START = 0x02,
    CMD_DATA = 0x03,
    CMD_END = 0x04,
    CMD_RESET = 0x05,

    CMD_ACK = 0x7F,
    CMD_NACK = 0x7E,
} serial_cmd_t;

bool recv_frame(serial_cmd_t* cmd, uint8_t** payload, size_t* len);

void send_ack();

void send_nack();