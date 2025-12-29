#pragma once

#include "serial_flasher.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Check if a valid serial API has been set.
 *
 * This function verifies that all required function pointers in the
 * serial API structure are properly assigned and ready for use
 * by the serial flasher state machine.
 *
 * @return true If a valid API is set.
 * @return false If the API is incomplete or invalid.
 */
bool check_valid_api(void);

/**
 * @brief Get the currently set serial API.
 *
 * This function returns a pointer to the serial API structure that
 * has been previously set using `set_serial_api()`.
 *
 * @return serial_api_t* Pointer to the currently set serial API.
 */
serial_api_t* get_serial_api(void);
