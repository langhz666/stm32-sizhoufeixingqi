/**
 * @file    Int_IP5305T.c
 * @brief   IP5305T电源管理实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过GPIO控制电源按键实现自动开关机
 */

#include "Int_IP5305T.h"

/**
 * @brief 启动IP5305T电源
 * @note  模拟短按电源键: 拉低100ms后释放
 */
void Int_IP5305T_start(void)
{
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}

/**
 * @brief 执行关机指令
 * @note  模拟双击电源键: 短按->等待->短按
 */
void Int_IP5305T_shutdown(void)
{
    /* 第一次短按 */
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
    vTaskDelay(200);

    /* 第二次短按 */
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}
