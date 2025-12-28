#include "uart_handler.h"
#include "main.h"

extern UART_HandleTypeDef huart1;

#define RX_BUFFER_SIZE (1*1024)
static uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rxHead = 0;
volatile uint16_t rxTail = 0;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) // Check which UART triggered
    {
        // Increment head
        rxHead = (rxHead + 1) % RX_BUFFER_SIZE;

        // Restart reception
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&rxBuffer[rxHead], 1);
    }
}

uint8_t UART_Available(void)
{
    return (rxHead != rxTail);
}


uint8_t UART_ReadByte(void)
{
    uint8_t data = 0;
    if (rxHead != rxTail)
    {
        data = rxBuffer[rxTail];
        rxTail = (rxTail + 1) % RX_BUFFER_SIZE;
    }
    return data;
}

int uart1_send(const uint8_t* buf, size_t len){
   const HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
   return (ret == HAL_OK? (int)(len):-1);
}

int uart1_recv(uint8_t *buf, size_t len, uint32_t timeout_ms)
{
    size_t pivot = 0;
    uint32_t tickstart = HAL_GetTick(); // Current time in ms

    while (pivot < len)
    {
        // Check if a byte is available in ring buffer
        if (UART_Available())
        {
            buf[pivot++] = UART_ReadByte();
        }
        else
        {
            // No data available, check timeout
            if ((HAL_GetTick() - tickstart) >= timeout_ms)
            {
                break; // Timeout expired
            }
        }
    }
    return pivot; // Number of bytes actually read
}

int uart_start_it(){
    return UART_Start_Receive_IT(&huart1, (uint8_t *)&rxBuffer[rxHead], 1);
}
