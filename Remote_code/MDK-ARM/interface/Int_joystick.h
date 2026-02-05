#ifndef __INT_JOYSTICK__
#define __INT_JOYSTICK__

#include "adc.h"

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
} Joystick_Struct;

/**
 * @brief 初始化ADC遥控  打开ADC
 *
 */
void Int_joystick_init(void);

/**
 * @brief 读取ADC数据 保存到结构体地址中
 * 
 * @param joystick 
 */
void Int_joystick_get(Joystick_Struct *joystick);

#endif // __INT_JOYSTICK__
