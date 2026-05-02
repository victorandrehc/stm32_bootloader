/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "stm32f4xx_it.h"

#include "main.h"

#include "boot_config.h"
#include <stdio.h>

// volatile bootloader_api_t* bootloader_api_ptr = (bootloader_api_t*) BOOT_CONFIG_START_ADDR;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    while (1)
    { }
}

extern void hardfault_c(uint32_t* fault_sp);
void hardfault_c_internal(uint32_t* fault_sp)
{
    hardfault_c(fault_sp);
}


/**
 * @brief This function handles Hard fault interrupt.
 */
__attribute__((naked, used)) void HardFault_Handler(void)
{
    __asm volatile(
     "tst lr,#4 \n"
     "ite eq \n"
     "mrseq r0, msp \n"
     "mrsne r0, psp \n"
     "bl hardfault_c_internal \n"
    );
    while (1)
    { }
}



/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    while (1)
    { }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    while (1)
    { }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    while (1)
    { }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) { }

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) { }

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) { }

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

void EXTI15_10_IRQHandler(void)
{
    /* Check if EXTI line is pending for B1_Pin */
    HAL_GPIO_EXTI_IRQHandler(B1_Pin);
}
