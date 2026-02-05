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
 * @brief 飞控任务初始化 MPU6050初始化    启动电机
 * 
 */
void App_flight_init(void);

/**
 * @brief 根据陀螺仪测量的数据 计算出欧拉角
 *
 */
void App_flight_get_euler_angle(void);

/**
 * @brief 根据欧拉角 计算出PID的目标值
 *
 */
void App_flight_pid_process(void);


/**
 * @brief 根据PID的输出值 控制电机
 *
 */
void App_flight_control_motor(void);


/**
 * @brief 进入定高功能之后的PID计算
 * 
 */
void App_flight_fix_height_pid_process(void);

#endif // __APP_FLIGHT__
