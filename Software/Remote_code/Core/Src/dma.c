/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma.c
  * @brief   DMA控制器配置与初始化
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "dma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* DMA配置                                                                     */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief  DMA控制器初始化函数
 *
 * @details 配置DMA1控制器：
 *          - 使能DMA1时钟
 *          - 配置DMA1_Channel1中断（用于ADC1数据传输）
 *
 * DMA通道分配：
 * - DMA1_Channel1：ADC1 -> 内存（摇杆数据采集）
 *
 * @note   此函数由STM32CubeMX自动生成
 * @note   DMA中断优先级：抢占0，子优先级0
 */
void MX_DMA_Init(void)
{

  /* 使能DMA1控制器时钟 */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA中断配置 */
  /* DMA1_Channel1中断配置（ADC1数据传输） */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
