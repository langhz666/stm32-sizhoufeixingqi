/**
 * @file    Int_motor.c
 * @brief   电机驱动实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过PWM控制无刷电机转速
 */

#include "Int_motor.h"

/**
 * @brief 设置电机速度
 * @param motor 电机结构体指针
 * @note  实际设置的是PWM比较值，最大值1000
 */
void Int_motor_set_speed(Motor_Struct *motor)
{
    if (motor->speed > 1000)
    {
        debug_printf("motor speed is too big\r\n");
        return;
    }

    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);
}

/**
 * @brief 启动电机
 * @param motor 电机结构体指针
 * @note  先将PWM输出设为0，再启动PWM
 */
void Int_motor_start(Motor_Struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, 0);
    HAL_TIM_PWM_Start(motor->tim, motor->channel);
}
