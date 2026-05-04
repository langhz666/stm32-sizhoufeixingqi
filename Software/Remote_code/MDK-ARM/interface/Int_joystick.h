/**
 * @file    Int_joystick.h
 * @brief   摇杆ADC采集模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了摇杆ADC采集模块的公共接口：
 *          - Joystick_Struct数据结构
 *          - Int_joystick_init()函数声明
 *          - Int_joystick_get()函数声明
 *
 * ADC配置：
 * - 分辨率：12位（0-4095）
 * - 模式：连续扫描 + DMA
 * - 通道数：4
 *
 * 通道映射：
 * - CH1 (PA1): THR（油门）
 * - CH6 (PA6): YAW（偏航）
 * - CH2 (PA2): PIT（俯仰）
 * - CH3 (PA3): ROL（横滚）
 *
 * 使用示例：
 * @code
 * // 初始化
 * Int_joystick_init();
 *
 * // 获取数据
 * Joystick_Struct joystick;
 * Int_joystick_get(&joystick);
 * int16_t throttle = joystick.thr;
 * @endcode
 *
 * @note   数据范围：0-4095（12位ADC）
 * @note   DMA自动更新，无需手动触发转换
 */

#ifndef __INT_JOYSTICK__
#define __INT_JOYSTICK__

/* ======================== 头文件引用 ======================== */

#include "adc.h"    /* ADC驱动 */

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 摇杆数据结构
 *
 * 存储4通道ADC原始值：
 * - thr: 油门（Throttle）
 * - yaw: 偏航（Yaw）
 * - pit: 俯仰（Pitch）
 * - rol: 横滚（Roll）
 *
 * 数据范围：0-4095（12位ADC）
 * 后续处理：映射到0-1000控制范围
 *
 * @note   数据由DMA自动更新
 * @note   在多任务环境下建议在临界区内访问
 */
typedef struct
{
    int16_t thr;    /**< 油门轴：PA1 -> ADC1_IN1 */
    int16_t yaw;    /**< 偏航轴：PA6 -> ADC1_IN6 */
    int16_t pit;    /**< 俯仰轴：PA2 -> ADC1_IN2 */
    int16_t rol;    /**< 横滚轴：PA3 -> ADC1_IN3 */
} Joystick_Struct;

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  初始化摇杆ADC（DMA模式）
 *
 * @details 启动ADC1的DMA连续转换：
 *          - 4通道扫描模式
 *          - 连续转换使能
 *          - DMA循环模式
 *          - 数据右对齐
 *
 * @note   初始化后ADC开始自动转换
 * @note   adc_buff[]由DMA自动更新
 * @note   调用Int_joystick_get()获取最新值
 */
void Int_joystick_init(void);

/**
 * @brief  获取摇杆ADC值
 *
 * @param  joystick 指向Joystick_Struct的指针，用于存储结果
 *
 * @details 从DMA缓冲区读取最新ADC值：
 *          - adc_buff[0] -> joystick->thr
 *          - adc_buff[1] -> joystick->yaw
 *          - adc_buff[2] -> joystick->pit
 *          - adc_buff[3] -> joystick->rol
 *
 * @note   DMA在后台自动更新adc_buff[]
 * @note   读取操作是原子的（16位读取）
 * @note   数据范围：0-4095（12位ADC）
 */
void Int_joystick_get(Joystick_Struct *joystick);

#endif /* __INT_JOYSTICK__ */
