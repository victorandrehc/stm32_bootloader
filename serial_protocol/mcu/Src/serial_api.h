#pragma once

#include "serial_protocol.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


bool check_valid_api();

serial_api_t* get_serial_api();
