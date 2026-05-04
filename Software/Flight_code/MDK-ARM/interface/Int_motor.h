/**
 * @file    Int_motor.h
 * @brief   电机驱动模块 - PWM控制四个无刷电机
 * @author  langhz666
 * @date    2025-09-27
 * @note    使用TIM1/TIM2/TIM3/TIM4的PWM通道控制四个电机
 *          PWM比较值范围: 0-1000
 */

#ifndef __INT_MOTOR__
#define __INT_MOTOR__

#include "tim.h"
#include "Com_debug.h"

/**
 * @brief 电机控制结构体
 */
typedef struct
{
    TIM_HandleTypeDef *tim;     /**< 定时器句柄指针 */
    uint16_t channel;           /**< PWM通道 (TIM_CHANNEL_x) */
    int16_t speed;              /**< 电机速度 (0-1000, PWM比较值) */
} Motor_Struct;

/**
 * @brief 设置电机速度
 * @param motor 电机结构体指针
 * @note  speed范围: 0-1000, 超过1000会打印警告并返回
 */
void Int_motor_set_speed(Motor_Struct *motor);

/**
 * @brief 启动电机
 * @param motor 电机结构体指针
 * @note  初始化PWM输出，速度设为0
 */
void Int_motor_start(Motor_Struct *motor);

#endif /* __INT_MOTOR__ */
