/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  *          EXTI0 and EXTI15_10 handlers removed — not used in bring-up build.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include <stdio.h>

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/

void NMI_Handler(void)
{
    while (1) {}
}

void HardFault_Handler(void)
{
    printf("\r\n!!! HARD FAULT !!!\r\n");
    while (1) {}
}

void MemManage_Handler(void)
{
    printf("\r\n!!! MEM MANAGE FAULT !!!\r\n");
    while (1) {}
}

void BusFault_Handler(void)
{
    printf("\r\n!!! BUS FAULT !!!\r\n");
    while (1) {}
}

void UsageFault_Handler(void)
{
    printf("\r\n!!! USAGE FAULT !!!\r\n");
    while (1) {}
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/******************************************************************************/

/* EXTI0_IRQHandler and EXTI15_10_IRQHandler removed.                        */
/* H_EXTI_0 and B1_Pin EXTI no longer used in this build.                    */
/* USART2_IRQHandler is defined in main.c.                                    */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */