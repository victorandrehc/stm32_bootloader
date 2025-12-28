#include "serial_protocol.h"
#include "serial_process_frame.h"
#include "serial_api.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*

| Command | Direction  | Description              |
| ------- | ---------- | ------------------------ |
| PING    | Host → MCU | Check bootloader alive   |
| START   | Host → MCU | Begin update (size, CRC) |
| DATA    | Host → MCU | Firmware chunk           |
| END     | Host → MCU | Finish & verify          |
| RESET   | Host → MCU | Reboot into app          |
| ACK     | MCU → Host | Command OK               |
| NACK    | MCU → Host | Error code               |

*/



typedef enum {
    PING_STATE,
    START_STATE,
    DATA_STATE,
    END_STATE,
    RESET_STATE,
} serial_state_t;

static const char* get_serial_state_str(const serial_state_t serial_state){
    switch(serial_state){
        case PING_STATE:
            return "PING_STATE";
        case START_STATE:
            return "START_STATE";
        case DATA_STATE:
            return "DATA_STATE";
        case END_STATE:
            return "END_STATE";
        case RESET_STATE:
            return "RESET_STATE";
        default:
            return "UNKNOWN_STATE";
    }
}


static size_t get_fw_size(uint8_t* payload){
    //host and target have the same endianess
    const size_t* fw_size = (const size_t*) payload;
    return *fw_size;
}

static uint32_t get_fw_crc(uint8_t* payload){
    //host and target have the same endianess
    const uint32_t* fw_crc = (const uint32_t*)(payload + 4);
    return *fw_crc;
}

serial_state_t process_ping_state()
{
    int ret = 0;
    serial_cmd_t cmd = CMD_UNKNOWN;
    size_t len = 0;
    uint8_t* payload;
    ret = recv_frame(&cmd, &payload, &len);

    if (!ret){
        send_nack();
        return PING_STATE;
    }
    
    switch(cmd)
    {
        case CMD_PING:
            send_ack();
            return START_STATE;
        default:
            send_nack();
            return PING_STATE;
    }
    
}

serial_state_t process_start_state(size_t* fw_size, uint16_t* fw_crc)
{
    int ret = 0;
    serial_cmd_t cmd = CMD_UNKNOWN;
    size_t len = 0;
    uint8_t* payload;
    ret = recv_frame(&cmd, &payload, &len);
    if (!ret){
        send_nack();
        return RESET_STATE;
    }

    serial_api_t* serial_api = get_serial_api();
    
    switch(cmd)
    {
        case CMD_START:
            serial_api->flash_reset();
            *fw_size = get_fw_size(payload);
            *fw_crc = get_fw_crc(payload);
            printf("fw_size 0x%x, max_fw_size 0x%x, crc 0x%lx\n",*fw_size,serial_api->max_fw_size,*fw_crc);
            if(*fw_size > serial_api->max_fw_size)
            {
                send_nack();
                return RESET_STATE;
            }
            send_ack();
            return DATA_STATE;
        default:
            send_nack();
            return RESET_STATE;
    }
}

serial_state_t process_data_state(size_t fw_size, uint16_t fw_crc)
{
    int ret = 0;
    serial_cmd_t cmd = CMD_UNKNOWN;
    size_t len = 0;
     uint8_t* payload;
    ret = recv_frame(&cmd, &payload, &len);
    if (!ret){
        send_nack();
        return RESET_STATE;
    }
    serial_api_t* serial_api = get_serial_api();

    switch(cmd){
        case CMD_DATA:
            serial_api->flash_feed(payload,len);
            send_ack();
            // todo save on flash
            return DATA_STATE;
        case CMD_END: 
            serial_api->flash_flush();
            const bool ret = serial_api->fw_crc_check(fw_crc, fw_size);
            if(ret)
            {
                send_ack();
                return END_STATE;
            }
            send_nack();
            return RESET_STATE;
        default:
            send_nack();
            return RESET_STATE;
    }

}


int recv_firmware(){
    if(!check_valid_api())
    {
        return -1;
    }
    serial_state_t serial_state = PING_STATE;
    size_t fw_size = 0;
    uint16_t fw_crc = 0;

    while(serial_state != END_STATE){
        serial_state_t next_serial_state = serial_state;
        switch(serial_state){
            case PING_STATE:
                fw_size = 0;
                fw_crc = 0;
                next_serial_state = process_ping_state();
                break;
            case START_STATE:

                next_serial_state = process_start_state(&fw_size, &fw_crc);
                break;
            case DATA_STATE:
                next_serial_state = process_data_state(fw_size, fw_crc);
                break;
            case END_STATE:
                
                break;
            case RESET_STATE:
            default:
                next_serial_state = PING_STATE;
                break;
        }
        if(next_serial_state != serial_state){
            printf("Serial state change, new state: %s[%d]->%s[%d]\n",get_serial_state_str(serial_state), serial_state,
            get_serial_state_str(next_serial_state), next_serial_state);
        }
        serial_state = next_serial_state;
    }
    return 0;
}


