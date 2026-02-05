/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_LEFT_X_Pin GPIO_PIN_2
#define KEY_LEFT_X_GPIO_Port GPIOB
#define KEY_RIGHT_X_Pin GPIO_PIN_10
#define KEY_RIGHT_X_GPIO_Port GPIOB
#define KEY_UP_Pin GPIO_PIN_11
#define KEY_UP_GPIO_Port GPIOB
#define KEY_RIGHT_Pin GPIO_PIN_12
#define KEY_RIGHT_GPIO_Port GPIOB
#define KEY_LEFT_Pin GPIO_PIN_13
#define KEY_LEFT_GPIO_Port GPIOB
#define KEY_DOWN_Pin GPIO_PIN_14
#define KEY_DOWN_GPIO_Port GPIOB
#define POWER_KEY_Pin GPIO_PIN_15
#define POWER_KEY_GPIO_Port GPIOB
#define SPI1_NSS_Pin GPIO_PIN_15
#define SPI1_NSS_GPIO_Port GPIOA
#define SI_EN_Pin GPIO_PIN_7
#define SI_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
