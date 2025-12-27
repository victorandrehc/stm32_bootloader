#include "serial_protocol.h"
#include "serial_process_frame.h"
#include <stdbool.h>

static serial_api_t serial_uart_api;

void set_serial_api(uart_send send, uart_recv recv){
    serial_uart_api.send = send;
    serial_uart_api.recv = recv;
}

bool check_valid_api()
{
    return  serial_uart_api.send != NULL &&  serial_uart_api.recv != NULL;
}

serial_api_t* get_serial_api(){
    return &serial_uart_api;
}