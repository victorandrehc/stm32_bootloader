#include "main.h"
#include "boot_config.h"
#include "uart_handler.h"
#include "flash_handler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <serial_protocol.h>

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);




void blink(int delay_ms){
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  HAL_Delay(delay_ms);
}


static volatile bool button_pressed = false;
bool try_enter_DFU_mode(){
  button_pressed = false;
  bool dfu = (bootloader_api_ptr->boot_info.magic == BOOT_INFO_MAGIC && (reset_reason_e)bootloader_api_ptr->boot_info.reset_reason_uint == FIRMWARE_UPDATE);
  const uint16_t delay_ms = 1000u;
  const int num_tries = 3;
  for(int i =0; i < num_tries && !dfu; i++){
    printf("Booting in %i ms\n",(num_tries - i)*delay_ms);
    dfu|=button_pressed;
    HAL_Delay(delay_ms);
  }
  return dfu;
}


int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  
  uart_start_it();
  init_boot_api();
  
  printf("   ____              __\n");
  printf("  / __ )____  ____  / /_\n");
  printf(" / __  / __ \\/ __ \\/ __/\n");
  printf("/ /_/ / /_/ / /_/ / /_\n");
  printf("/_____/\\____/\\____/\\__/\n");
  printf("\n");
  printf("Press Button to Enter DFU mode\n");
  const char* reboot_reason = get_reset_reason_string();
  printf("MAGIC NUMBER: 0x%08lx RESET_REASON: %s\n",bootloader_api_ptr->boot_info.magic, reboot_reason);


  bool dfu = try_enter_DFU_mode();

  if(dfu)
  {
    printf("Entering in DFU ...\n");
    bootloader_api_ptr->boot_info.reset_reason_uint = POWER_CYCLE;
    const size_t max_fw_size = get_max_fw_size();
    serial_api_t serial_api = {uart1_send, uart1_recv, flash_fw_feed, flash_fw_flush, flash_fw_reset, fw_crc_check, max_fw_size};
    set_serial_api(serial_api);
    recv_firmware();
    bootloader_api_ptr->reset(APPLICATION_RESET);
  }


 
  if(bootloader_api_ptr != (void*)(BOOT_CONFIG_START_ADDR))
  {
    printf("BootLoader API not in place\n");
    Error_Handler();
  }
  
  bootloader_api_ptr->jump_to_application();
  printf("SHOULD NOT HAVE RETURNED RESETING IN DFU\n");
  bootloader_api_ptr->reset(FIRMWARE_UPDATE);
  Error_Handler();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }__HAL_RCC_CRC_CLK_ENABLE();  // enable CRC clock
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;


  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);  
  HAL_NVIC_EnableIRQ(USART1_IRQn); 

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}




void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin != B1_Pin)
  {
    return;
  }
  button_pressed = true;
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    blink(1000);
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

