
#include "boot_config.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

#define BOOT_START_ADDR 0x8000000
#define APP_START_ADDR 0x08008000

static volatile FLASH_CFG bootloader_api_t bootloader_api;
volatile bootloader_api_t* bootloader_api_ptr = &bootloader_api; 

static void deinit_peripherals(void)
{
    // Stop SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // Reset SysTick interrupt
    NVIC_ClearPendingIRQ(SysTick_IRQn);

    HAL_RCC_DeInit();

    // Clear pending faults
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
    SCB->SHCSR = 0;
}

static void jump_to_address(uintptr_t app_addr)
{
    size_t app_stack = *(volatile size_t*)app_addr;
    size_t app_reset_handler = *(volatile size_t*)(app_addr + 4);
    printf("app_stack: 0x%x\tapp_reset_handler: 0x%x\n", app_stack, app_reset_handler);
    printf("Vector table @0x%08X: MSP=0x%08X, Reset=0x%08X\n",
       app_addr, app_stack, app_reset_handler);
    
    if((app_stack & 0x2FFE0000) != 0x20000000){
      printf("No application loaded\n");
      return;
    } 

    __disable_irq();
    deinit_peripherals();
    HAL_DeInit();
    SCB->VTOR = app_addr;
    __set_MSP(app_stack);
    __enable_irq();
    ((void(*)(void))app_reset_handler)();
}

static void jump_to_application_implementation(){
  jump_to_address(APP_START_ADDR);
}

static void jump_to_bootloader_implementaton(const reset_reason_e reset_reason){
  bootloader_api.boot_info.magic = BOOT_INFO_MAGIC;
  bootloader_api.boot_info.reset_reason_uint = (uint32_t) reset_reason;
  NVIC_SystemReset();
}




void init_boot_api(){
  bootloader_api.jump_to_application = jump_to_application_implementation;
  bootloader_api.reset = jump_to_bootloader_implementaton;
}

const char* get_reset_reason_string()
{
  if(bootloader_api.boot_info.magic != BOOT_INFO_MAGIC)
  {
    return "UNKNOWN_MAGIC_NUMBER_MISMATCH";
  }

  const reset_reason_e reset_reason = (reset_reason_e)bootloader_api.boot_info.reset_reason_uint;
  switch(reset_reason)
  {
    case POWER_CYCLE:
      return "POWER_CYCLE";
    case APPLICATION_RESET:
      return "APPLICATION RESET";
    case FIRMWARE_UPDATE:
      return "FIRMWARE UPDATE";
    default:
      break;
  }
  return "UNKNOWN_REASON";
}

