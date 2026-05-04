/**
 * @file    main.h
 * @brief   主程序头文件 - GPIO引脚定义和全局声明
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件包含：
 *          - STM32F1xx HAL库头文件引用
 *          - GPIO引脚宏定义
 *          - Error_Handler()函数声明
 *
 * GPIO引脚分配：
 * - PB2/PB10/PB11/PB12/PB13/PB14/PB15: 按键输入（低电平有效）
 * - PA15: SPI1_NSS（SI24R1片选，软件控制）
 * - PB7: SI_EN（SI24R1芯片使能）
 * - PA1/PA2/PA3/PA6: ADC输入（摇杆4通道）
 * - PA9/PA10: USART1 TX/RX（调试串口）
 * - PB0/PB1: OLED RST/DC
 * - PA4/PA5/PA7: OLED CS/SCLK/SDIN
 *
 * @note   此文件由STM32CubeMX自动生成，手动修改可能被覆盖
 * @note   用户代码应放在USER CODE BEGIN/END区域内
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"  /* STM32F1xx HAL库 */

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

/**
 * @brief  错误处理函数
 *
 * @details 禁用所有中断并进入无限循环：
 *          - 调用__disable_irq()禁用中断
 *          - 进入while(1)死循环
 *          - 系统必须复位才能恢复
 *
 * @note   通常在初始化失败时调用
 * @note   可在此函数中添加错误指示（如LED闪烁）
 */
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* ======================== 按键GPIO定义 ======================== */

/**
 * @name 方向按键（低电平有效，内部上拉）
 * @{
 */
#define KEY_UP_Pin              GPIO_PIN_11     /**< 上方向键 */
#define KEY_UP_GPIO_Port        GPIOB
#define KEY_DOWN_Pin            GPIO_PIN_14     /**< 下方向键 */
#define KEY_DOWN_GPIO_Port      GPIOB
#define KEY_LEFT_Pin            GPIO_PIN_13     /**< 左方向键 */
#define KEY_LEFT_GPIO_Port      GPIOB
#define KEY_RIGHT_Pin           GPIO_PIN_12     /**< 右方向键 */
#define KEY_RIGHT_GPIO_Port     GPIOB
/** @} */

/**
 * @name 功能按键（低电平有效，内部上拉）
 * @{
 */
#define KEY_LEFT_X_Pin          GPIO_PIN_2      /**< 左侧功能键 */
#define KEY_LEFT_X_GPIO_Port    GPIOB
#define KEY_RIGHT_X_Pin         GPIO_PIN_10     /**< 右侧功能键（支持长按） */
#define KEY_RIGHT_X_GPIO_Port   GPIOB
/** @} */

/**
 * @name 电源控制
 * @{
 */
#define POWER_KEY_Pin           GPIO_PIN_15     /**< IP5305T电源按键（低电平有效） */
#define POWER_KEY_GPIO_Port     GPIOB
/** @} */

/* ======================== SI24R1无线模块定义 ======================== */

/**
 * @name SI24R1控制引脚
 * @{
 */
#define SPI1_NSS_Pin            GPIO_PIN_15     /**< SPI片选（低电平有效，软件控制） */
#define SPI1_NSS_GPIO_Port      GPIOA
#define SI_EN_Pin               GPIO_PIN_7      /**< SI24R1芯片使能（高电平使能） */
#define SI_EN_GPIO_Port         GPIOB
/** @} */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
