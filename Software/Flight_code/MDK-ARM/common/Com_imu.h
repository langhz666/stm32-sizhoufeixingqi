/**
 * @file    Com_imu.h
 * @brief   IMU姿态解算模块 - 基于四元数的姿态估计
 * @author  langhz666
 * @date    2025-09-27
 * @note    使用Mahony互补滤波算法融合陀螺仪和加速度计数据
 */

#ifndef __COMMON_IMU_H
#define __COMMON_IMU_H

#include "Com_debug.h"
#include "Com_Config.h"
#include "math.h"

/**
 * @brief 四元数结构体
 */
typedef struct
{
    float q0;   /**< 四元数实部 */
    float q1;   /**< 四元数虚部x */
    float q2;   /**< 四元数虚部y */
    float q3;   /**< 四元数虚部z */
} Quaternion_Struct;

/* 常量定义 */
extern float RtA;      /**< 弧度转角度系数 (180/PI) */
extern float Gyro_G;   /**< 陀螺仪原始值转度/秒系数 */
extern float Gyro_Gr;  /**< 陀螺仪原始值转弧度/秒系数 */

/**
 * @brief 获取欧拉角 (姿态解算主函数)
 * @param gyroAccel 陀螺仪+加速度计原始数据指针
 * @param eulerAngle 输出的欧拉角指针
 * @param dt         积分时间 (秒)
 * @note  使用Mahony互补滤波算法:
 *        1. 用加速度计修正陀螺仪漂移
 *        2. 四元数积分更新姿态
 *        3. 四元数转欧拉角
 */
void Common_IMU_GetEulerAngle(Gyro_Accel_Struct *gyroAccel,
                              Euler_struct *eulerAngle,
                              float dt);

/**
 * @brief 获取Z轴合成加速度
 * @return Z轴加速度值
 * @note   考虑机体倾斜时的Z轴加速度合成
 */
float Common_IMU_GetNormAccZ(void);

#endif /* __COMMON_IMU_H */
