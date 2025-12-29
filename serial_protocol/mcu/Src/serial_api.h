#pragma once

#include "serial_protocol.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool check_valid_api();

serial_api_t* get_serial_api();
