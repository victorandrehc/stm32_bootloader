#include "serial_protocol.h"
#include "serial_process_frame.h"
#include "serial_api.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
Prototol Package
+--------+--------+--------+--------+--------+--------+--------+------+
| SOF    | VER    | CMD    | LEN_L  | LEN_H  | PAYLOAD| CRC_L  | CRC_H|
+--------+--------+--------+--------+--------+--------+--------+------+
 1 byte   1 byte   1 byte   1 byte   1 byte    N bytes   1 byte  1 byte

- SOF: 0xA5

- VER: protocol version (start with 0x01)

- CMD: command ID

- LEN: payload length (little-endian)

- CRC: CRC16-CCITT over VER..PAYLOAD
*/

#define SOF 0xA5
#define VERSION 0x01
#define BUFFER_MAX_SIZE 1024
#define TIMEOUT_MS 100


static uint8_t buffer[BUFFER_MAX_SIZE];



static void print_frame(const uint8_t* frame, size_t len){
    printf("Frame Received: ");
    for(size_t i =0;i<len;i++){
        printf("0x%x ", frame[i]);
    }
    printf("\n");
}

static uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}


bool recv_frame(serial_cmd_t *cmd, uint8_t **payload, size_t *len){
    if(!check_valid_api()){
        return false;
    }
    serial_api_t* serial_api = get_serial_api();
    // seach for header
    uint8_t* header = buffer;
    size_t pivot = 0;
    int ret = -1;
    while(pivot<5){
        ret = serial_api->recv(&header[pivot], 1, TIMEOUT_MS);
        if(ret<=0){
           return false;
        }
        if(pivot == 0 && header[pivot] != SOF){
            continue;
        }
        pivot++;
    }
    
    // print_frame(header,5);
    *cmd = header[2];
    *len = header[3] | (header[4] << 8);
    printf("SOF:%x, ver %x, cmd %x, len %x\n",header[0], header[1], *cmd, *len);

    //remove header space and crc space
    if(*len > BUFFER_MAX_SIZE - 5 - 2){
        printf("Payload Bigger than allocated buffer\n");
        return false;
    }
    
    if(*len > 0 )
    {
        uint8_t* payload_internal = &buffer[5];
        *payload = payload_internal;
        ret = serial_api->recv(payload_internal, *len, TIMEOUT_MS);
        if(ret<=0){
            printf("Timeout on recevining payload");
            return false;
        }
        
        print_frame(payload_internal,*len);
    }

    uint8_t* crc_buffer = &buffer[5 + (*len)];
    ret = serial_api->recv(crc_buffer, 2, TIMEOUT_MS);
     if(ret<=0){
        printf("Timeout on recevining crc");
        return -1;
    }
    // print_frame(crc_buffer,2);
    uint16_t crc_recv = crc_buffer[0] | (crc_buffer[1] <<8 );
    size_t buffer_crc_size = *len + 4;
    uint16_t crc_calc = crc16_ccitt(&buffer[1], buffer_crc_size);
    // print_frame(&buffer[1], buffer_crc_size);
    // printf("crc_calc: %0x, crc_recv: %x\n",crc_calc,crc_recv);
    return crc_calc == crc_recv;
}

static void send_ack_or_nack(serial_cmd_t cmd) {
    if(!check_valid_api()){
        return;
    }
    serial_api_t* serial_api = get_serial_api();
    uint8_t frame[] = {SOF, VERSION, cmd, 0, 0, 0x00, 0x00};
    uint16_t crc = crc16_ccitt(&frame[1], 4);
    frame[5] = crc & 0xFF;
    frame[6] = crc >> 8;
    serial_api->send(frame, sizeof(frame));
}

void send_ack(){
    send_ack_or_nack(CMD_ACK);
}

void send_nack(){
    send_ack_or_nack(CMD_NACK);
}
