/**
 * @file    Com_filter.h
 * @brief   滤波算法模块 - 提供低通滤波和卡尔曼滤波
 * @author  langhz666
 * @date    2025-09-27
 * @note    低通滤波用于陀螺仪数据，卡尔曼滤波用于加速度计数据
 */

#ifndef __COMMON_FILTER_H
#define __COMMON_FILTER_H

#include "Com_debug.h"

/**
 * @brief 卡尔曼滤波器结构体
 */
typedef struct
{
    float LastP;    /**< 上一时刻的状态方差 (预测协方差) */
    float Now_P;    /**< 当前时刻的状态方差 (预测协方差) */
    float out;      /**< 滤波器输出值 (最优估计状态) */
    float Kg;       /**< 卡尔曼增益 (用于计算预测值和测量值之间的权重) */
    float Q;        /**< 过程噪声的方差 (反映系统模型的不确定性) */
    float R;        /**< 测量噪声的方差 (反映测量过程的不确定性) */
} KalmanFilter_Struct;

/* 卡尔曼滤波器实例 (X/Y/Z三轴) */
extern KalmanFilter_Struct kfs[3];

/**
 * @brief 一阶低通滤波
 * @param newValue          新采样值
 * @param preFilteredValue  上一次滤波后的值
 * @return  滤波后的值
 * @note    output = alpha * newValue + (1-alpha) * preFilteredValue
 */
int16_t Common_Filter_LowPass(int16_t newValue, int16_t preFilteredValue);

/**
 * @brief 卡尔曼滤波
 * @param kf    卡尔曼滤波器结构体指针
 * @param input 输入测量值
 * @return  滤波后的最优估计值
 */
double Common_Filter_KalmanFilter(KalmanFilter_Struct *kf, double input);

#endif /* __COMMON_FILTER_H */
