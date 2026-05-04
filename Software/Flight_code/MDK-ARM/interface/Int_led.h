/**
 * @file    Int_led.h
 * @brief   LED驱动模块 - 控制四个状态指示灯
 * @author  langhz666
 * @date    2025-09-27
 * @note    LED低电平点亮 (GPIO_PIN_RESET)
 *          前两个LED指示遥控器连接状态
 *          后两个LED指示飞行状态
 */

#ifndef __INT_LED__
#define __INT_LED__

#include "main.h"

/**
 * @brief LED控制结构体
 */
typedef struct
{
    GPIO_TypeDef *port;     /**< GPIO端口 */
    uint16_t pin;           /**< GPIO引脚 */
} LED_Struct;

/**
 * @brief 点亮LED
 * @param led LED结构体指针
 * @note  设置GPIO为低电平
 */
void Int_led_turn_on(LED_Struct *led);

/**
 * @brief 关闭LED
 * @param led LED结构体指针
 * @note  设置GPIO为高电平
 */
void Int_led_turn_off(LED_Struct *led);

/**
 * @brief 翻转LED状态
 * @param led LED结构体指针
 */
void Int_led_toggle(LED_Struct *led);

#endif /* __INT_LED__ */
