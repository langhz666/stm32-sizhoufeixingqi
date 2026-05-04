/**
 * @file    Int_led.c
 * @brief   LED驱动实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    LED低电平点亮
 */

#include "Int_led.h"

/**
 * @brief 点亮LED
 * @param led LED结构体指针
 */
void Int_led_turn_on(LED_Struct *led)
{
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭LED
 * @param led LED结构体指针
 */
void Int_led_turn_off(LED_Struct *led)
{
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
}

/**
 * @brief 翻转LED状态
 * @param led LED结构体指针
 */
void Int_led_toggle(LED_Struct *led)
{
    HAL_GPIO_TogglePin(led->port, led->pin);
}
