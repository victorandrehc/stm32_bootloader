#include "serial_protocol.h"
#include "serial_process_frame.h"
#include <stdbool.h>

static serial_api_t serial_api;

void set_serial_api(uart_send_t send, uart_recv_t recv, flash_feed_t flash_feed,
    flash_flush_t flash_flush, flash_reset_t flash_reset){
    serial_api.send = send;
    serial_api.recv = recv;
    serial_api.flash_feed = flash_feed;
    serial_api.flash_flush = flash_flush;
    serial_api.flash_reset = flash_reset;
}

bool check_valid_api()
{
    return  serial_api.send != NULL &&  serial_api.recv != NULL 
    && serial_api.flash_feed != NULL && serial_api.flash_flush != NULL && serial_api.flash_reset != NULL;
}

serial_api_t* get_serial_api(){
    return &serial_api;
}