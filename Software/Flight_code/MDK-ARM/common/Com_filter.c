/**
 * @file    Com_filter.c
 * @brief   滤波算法实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    低通滤波用于陀螺仪，卡尔曼滤波用于加速度计
 */

#include "Com_filter.h"

/**
 * @brief 低通滤波权重系数
 * @note  值越小，滤波效果越强，但响应越慢
 */
#define ALPHA 0.15

/**
 * @brief 一阶低通滤波实现
 * @param newValue          新采样值
 * @param preFilteredValue  上一次滤波后的值
 * @return  滤波后的值
 * @note    简化的IIR低通滤波器，本质是指数加权移动平均
 *          output = alpha * newValue + (1-alpha) * preFilteredValue
 */
int16_t Common_Filter_LowPass(int16_t newValue, int16_t preFilteredValue)
{
    return ALPHA * newValue + (1 - ALPHA) * preFilteredValue;
}

/**
 * @brief 卡尔曼滤波器实例 (X/Y/Z三轴)
 * @note  Q: 过程噪声, R: 测量噪声
 *        参数需要根据实际传感器特性调整
 */
KalmanFilter_Struct kfs[3] = {
    {0.02, 0, 0, 0, 0.001, 0.543},  /* X轴 */
    {0.02, 0, 0, 0, 0.001, 0.543},  /* Y轴 */
    {0.02, 0, 0, 0, 0.001, 0.543}   /* Z轴 */
};

/**
 * @brief 卡尔曼滤波实现
 * @param kf    卡尔曼滤波器结构体指针
 * @param input 输入测量值
 * @return  滤波后的最优估计值
 * @note    卡尔曼滤波五步:
 *          1. 预测状态: x(k|k-1) = x(k-1|k-1)
 *          2. 预测协方差: P(k|k-1) = P(k-1|k-1) + Q
 *          3. 计算卡尔曼增益: K = P(k|k-1) / (P(k|k-1) + R)
 *          4. 更新估计: x(k|k) = x(k|k-1) + K * (z(k) - x(k|k-1))
 *          5. 更新协方差: P(k|k) = (1-K) * P(k|k-1)
 */
double Common_Filter_KalmanFilter(KalmanFilter_Struct *kf, double input)
{
    /* 1. 预测协方差 */
    kf->Now_P = kf->LastP + kf->Q;

    /* 2. 计算卡尔曼增益 */
    kf->Kg = kf->Now_P / (kf->Now_P + kf->R);

    /* 3. 更新最优估计 */
    kf->out = kf->out + kf->Kg * (input - kf->out);

    /* 4. 更新协方差 */
    kf->LastP = (1 - kf->Kg) * kf->Now_P;

    return kf->out;
}
