/**
 * @file    App_flight.h
 * @brief   飞行控制应用层 - 姿态解算、PID控制、电机输出
 * @author  langhz666
 * @date    2025-09-27
 * @note    实现四旋翼飞行器的核心控制逻辑:
 *          1. 读取MPU6050数据并解算欧拉角
 *          2. 串级PID控制 (外环角度 + 内环角速度)
 *          3. 混控输出到四个电机
 */

#ifndef __APP_FLIGHT__
#define __APP_FLIGHT__

#include "math.h"
#include "Com_debug.h"
#include "Com_filter.h"
#include "Com_imu.h"
#include "Com_pid.h"
#include "Int_motor.h"
#include "Int_mpu6050.h"
#include "Int_VL53L1X.h"

/**
 * @brief 飞控应用初始化
 * @note  初始化MPU6050、电机、VL53L1X
 */
void App_flight_init(void);

/**
 * @brief 获取欧拉角 (姿态解算)
 * @note  读取MPU6050数据 -> 滤波 -> 计算欧拉角
 */
void App_flight_get_euler_angle(void);

/**
 * @brief PID控制计算
 * @note  根据遥控器指令和当前姿态，计算PID输出
 *        串级PID: 角度环(外环) + 角速度环(内环)
 */
void App_flight_pid_process(void);

/**
 * @brief 电机控制输出
 * @note  根据PID输出和飞行状态，混控四个电机
 *        混控公式: 电机 = 油门 ± 俯仰 ± 横滚 ± 偏航
 */
void App_flight_control_motor(void);

/**
 * @brief 定高PID计算
 * @note  使用VL53L1X激光测距传感器实现定高控制
 *        24ms执行一次 (激光传感器采样周期)
 */
void App_flight_fix_height_pid_process(void);

#endif /* __APP_FLIGHT__ */
