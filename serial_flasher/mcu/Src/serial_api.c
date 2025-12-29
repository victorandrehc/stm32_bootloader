#include "serial_flasher.h"
#include "serial_process_frame.h"

#include <stdbool.h>

static serial_api_t serial_api = NULL;

void set_serial_api(serial_api_t api)
{
    serial_api = api;
}

bool check_valid_api()
{
    return serial_api.send != NULL && serial_api.recv != NULL && serial_api.flash_feed != NULL && serial_api.flash_flush != NULL
           && serial_api.flash_reset != NULL;
}

serial_api_t* get_serial_api()
{
    return &serial_api;
}