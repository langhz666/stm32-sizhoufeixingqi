/**
 * @file    Com_pid.h
 * @brief   PID控制算法模块 - 支持单级PID和串级PID
 * @author  langhz666
 * @date    2025-09-27
 * @note    串级PID用于姿态控制: 外环(角度) + 内环(角速度)
 */

#ifndef __COM_PID__
#define __COM_PID__

#include "main.h"

/**
 * @brief PID控制周期 (秒)
 * @note  对应飞控任务周期6ms
 */
#define PID_PERIOD 0.006

/**
 * @brief PID控制器结构体
 * @note  若CPU性能足够，建议使用double类型提高精度
 */
typedef struct
{
    float kp;       /**< 比例系数 - 值越大响应速度越快 */
    float ki;       /**< 积分系数 - 消除静态误差，一般不使用 */
    float kd;       /**< 微分系数 - 值越大抑制效果越强，但会放大噪声 */
    float err;      /**< 当前误差 (measure - desire) */
    float desire;   /**< 目标值 */
    float measure;  /**< 测量值 */
    float last_err; /**< 上一次误差 */
    float integral; /**< 误差累积量 */
    float output;   /**< PID输出值 */
} PID_Struct;

/**
 * @brief 单级PID计算
 * @param pid   PID结构体指针
 * @note  output = kp*err + ki*integral*dt + kd*der/dt
 */
void Com_PID_Calc(PID_Struct *pid);

/**
 * @brief 串级PID计算 (外环+内环)
 * @param out_pid   外环PID结构体指针 (角度环)
 * @param in_pid    内环PID结构体指针 (角速度环)
 * @note  外环输出作为内环目标值
 */
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid);

/**
 * @brief 限幅函数 - 将值限制在指定范围内
 * @param speed      输入值
 * @param max_speed  最大值
 * @param min_speed  最小值
 * @return  限幅后的值
 */
int16_t Com_limit(int16_t speed, int16_t max_speed, int16_t min_speed);

#endif /* __COM_PID__ */
