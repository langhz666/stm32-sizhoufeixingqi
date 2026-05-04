/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.h
  * @brief   中断处理函数声明头文件
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

/* 防止重复包含 */
#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/** @name Cortex-M3处理器异常处理函数
 * @{
 */
void NMI_Handler(void);             /**< 不可屏蔽中断 */
void HardFault_Handler(void);       /**< 硬件错误 */
void MemManage_Handler(void);       /**< 内存管理错误 */
void BusFault_Handler(void);        /**< 总线错误 */
void UsageFault_Handler(void);      /**< 用法错误 */
void DebugMon_Handler(void);        /**< 调试监视器 */
void SysTick_Handler(void);         /**< 系统滴答定时器 */
/** @} */

/** @name STM32外设中断处理函数
 * @{
 */
void DMA1_Channel1_IRQHandler(void);    /**< DMA1通道1中断（ADC1） */
/** @} */

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_IT_H */
