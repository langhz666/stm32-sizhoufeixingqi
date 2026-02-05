#ifndef __INT_LED__
#define __INT_LED__

#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} LED_Struct;

/**
 * @brief 打开LED灯
 *
 * @param led
 */
void Int_led_turn_on(LED_Struct *led);

/**
 * @brief 关闭LED灯
 * 
 * @param led 
 */
void Int_led_turn_off(LED_Struct *led);

/**
 * @brief 翻转LED灯
 * 
 * @param led 
 */
void Int_led_toggle(LED_Struct *led);

#endif // __INT_LED__
