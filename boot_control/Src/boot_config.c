
#include "boot_config.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

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



void jump_to_application(uintptr_t app_addr)
{
    size_t app_stack = *(volatile size_t*)app_addr;
    size_t app_reset_handler = *(volatile size_t*)(app_addr + 4);
    printf("app_stack: 0x%x\tapp_reset_handler: 0x%x\n", app_stack, app_reset_handler);
    printf("STACK VALIDITY: 0x%x\n", (app_stack & 0x2FFE0000));
    
    if((app_stack & 0x2FFE0000) != 0x20000000){
      printf("No application loaded\n");
    } 

    __disable_irq();
    deinit_peripherals();
    HAL_DeInit();
    SCB->VTOR = app_addr;
    __set_MSP(app_stack);
    __enable_irq();
    ((void(*)(void))app_reset_handler)();
}
