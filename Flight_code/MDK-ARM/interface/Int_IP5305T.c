#include "Int_IP5305T.h"

/**
 * @brief 启动IP5305T电源 避免自动关机
 *
 */
void Int_IP5305T_start(void)
{
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}

/**
 * @brief 软件执行关机指令
 *
 */
void Int_IP5305T_shutdown(void)
{
    // 1s中之内连续短按两次按钮 => 关机
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
    vTaskDelay(200);

    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}
