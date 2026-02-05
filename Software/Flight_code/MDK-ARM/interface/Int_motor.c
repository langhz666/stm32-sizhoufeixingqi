#include "Int_motor.h"

/**
 * @brief 传入的参数其实是比较值  最大为1000  默认值为200
 *
 * @param speed
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
 * @brief 启动电机 传入具体电机的结构体
 *
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel,0);
    HAL_TIM_PWM_Start(motor->tim, motor->channel);
}
