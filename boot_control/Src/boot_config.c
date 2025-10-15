
#include "boot_config.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

#define BOOT_START_ADDR 0x8000000
#define APP_START_ADDR 0x08008000

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

static void jump_to_bootloader_implementaton(){
  jump_to_address(BOOT_START_ADDR);
}

volatile FLASH_CFG bootloader_api_t bootloader_api = {
  .jump_to_application = jump_to_application_implementation,
  .jump_to_bootloader = jump_to_bootloader_implementaton,
};

