#include "Int_led.h"

/**
 * @brief 打开LED灯
 *
 * @param led
 */
void Int_led_turn_on(LED_Struct *led)
{
    // 直接修改引脚电平为低电平  开灯
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭LED灯
 *
 * @param led
 */
void Int_led_turn_off(LED_Struct *led)
{
    // 直接修改引脚电平为高电平  关灯
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
}

/**
 * @brief 翻转LED灯
 * 
 * @param led 
 */
void Int_led_toggle(LED_Struct *led)
{
    HAL_GPIO_TogglePin(led->port, led->pin);
}
