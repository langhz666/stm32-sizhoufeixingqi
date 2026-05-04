/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO引脚配置与初始化
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* GPIO配置                                                                    */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief  GPIO初始化函数
 *
 * @details 配置所有使用的GPIO引脚：
 *
 * GPIO时钟使能：
 * - GPIOC、GPIOD、GPIOA、GPIOB
 *
 * 按键引脚配置（输入，内部上拉）：
 * - PB2：KEY_LEFT_X（左侧功能键）
 * - PB10：KEY_RIGHT_X（右侧功能键）
 * - PB11：KEY_UP（上方向键）
 * - PB12：KEY_RIGHT（右方向键）
 * - PB13：KEY_LEFT（左方向键）
 * - PB14：KEY_DOWN（下方向键）
 *
 * 输出引脚配置：
 * - PB15：POWER_KEY（IP5305T电源按键，初始高电平）
 * - PA15：SPI1_NSS（SI24R1片选，初始高电平）
 * - PB7：SI_EN（SI24R1使能，初始低电平）
 *
 * @note   此函数由STM32CubeMX自动生成
 * @note   所有按键低电平有效，内部上拉
 */
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO端口时钟使能 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 设置输出引脚初始电平 */
  HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);      /* 电源按键初始高电平（释放） */
  HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET);       /* SPI片选初始高电平（未选中） */
  HAL_GPIO_WritePin(SI_EN_GPIO_Port, SI_EN_Pin, GPIO_PIN_RESET);           /* SI24R1使能初始低电平（禁用） */

  /* 配置按键引脚：输入模式，内部上拉 */
  GPIO_InitStruct.Pin = KEY_LEFT_X_Pin|KEY_RIGHT_X_Pin|KEY_UP_Pin|KEY_RIGHT_Pin
                          |KEY_LEFT_Pin|KEY_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* 配置电源按键和SI24R1使能引脚：推挽输出 */
  GPIO_InitStruct.Pin = POWER_KEY_Pin|SI_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* 配置SPI片选引脚：推挽输出 */
  GPIO_InitStruct.Pin = SPI1_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_NSS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
