/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   中断服务函数
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
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern void xPortSysTickHandler(void);
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3处理器中断与异常处理函数                                  */
/******************************************************************************/

/**
 * @brief  不可屏蔽中断处理函数
 *
 * @details NMI中断处理：
 *          - 进入无限循环
 *          - 系统必须复位才能恢复
 *
 * @note   NMI通常由硬件故障触发
 */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief  硬件错误中断处理函数
 *
 * @details HardFault处理：
 *          - 进入无限循环
 *          - 系统必须复位才能恢复
 *
 * @note   通常由非法内存访问、除零等触发
 * @note   调试时可通过调用栈定位错误位置
 */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief  内存管理错误中断处理函数
 *
 * @details MemManage处理：
 *          - 进入无限循环
 *          - 系统必须复位才能恢复
 *
 * @note   通常由MPU权限违规触发
 */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief  总线错误中断处理函数
 *
 * @details BusFault处理：
 *          - 进入无限循环
 *          - 系统必须复位才能恢复
 *
 * @note   通常由总线访问错误触发
 */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief  用法错误中断处理函数
 *
 * @details UsageFault处理：
 *          - 进入无限循环
 *          - 系统必须复位才能恢复
 *
 * @note   通常由未定义指令、非法状态等触发
 */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief  调试监视器中断处理函数
 *
 * @note   用于调试目的
 */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief  系统滴答定时器中断处理函数
 *
 * @details SysTick中断处理：
 *          1. 检查FreeRTOS调度器是否已启动
 *          2. 如果已启动，调用xPortSysTickHandler()处理FreeRTOS心跳
 *          3. 调用HAL_IncTick()更新HAL时基
 *
 * @note   SysTick中断频率：1000Hz（1ms周期）
 * @note   FreeRTOS需要此中断来驱动任务调度
 * @note   必须同时调用FreeRTOS和HAL的处理函数
 */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  /* 手动调用FreeRTOS的systick中断 */
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    /* 判断FreeRTOS是否已经启动 => 如果启动了, 调用FreeRTOS的systick中断 */
    xPortSysTickHandler();
  }

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx 外设中断处理函数                                                  */
/* 在此添加使用的外设中断处理函数                                                */
/* 可用的外设中断处理函数名称请参考启动文件（startup_stm32f1xx.s）                */
/******************************************************************************/

/**
 * @brief  DMA1通道1全局中断处理函数
 *
 * @details 处理ADC1的DMA传输完成中断：
 *          - 调用HAL_DMA_IRQHandler()处理DMA事件
 *          - ADC数据通过DMA自动传输到adc_buff[]
 *
 * @note   DMA1_Channel1用于ADC1数据采集
 * @note   循环模式下，中断会持续触发
 */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
