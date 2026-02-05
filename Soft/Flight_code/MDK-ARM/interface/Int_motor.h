#ifndef __INT_MOTOR__
#define __INT_MOTOR__

#include "tim.h"
#include "Com_debug.h"

typedef struct
{
    TIM_HandleTypeDef *tim;
    uint16_t channel;
    int16_t speed; // -32768 => 32767
} Motor_Struct;

/**
 * @brief 传入的参数其实是比较值  最大为1000  默认值为200
 *
 * @param speed
 */
void Int_motor_set_speed(Motor_Struct *motor);

/**
 * @brief 启动电机 传入具体电机的结构体
 *
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor);

#endif // __INT_MOTOR__
