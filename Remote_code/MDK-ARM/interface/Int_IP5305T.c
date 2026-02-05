#include "Int_IP5305T.h"


/**
 * @brief 启动IP5305T电源 避免自动关机
 * 
 */
void Int_IP5305T_start(void)
{
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port,POWER_KEY_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port,POWER_KEY_Pin, GPIO_PIN_SET);
}
