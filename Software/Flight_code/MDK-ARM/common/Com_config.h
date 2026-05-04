/**
 * @file    Com_config.h
 * @brief   公共配置文件 - 定义系统核心数据结构和枚举类型
 * @author  langhz666
 * @date    2025-09-27
 * @note    此文件包含飞控系统所有公共数据类型的定义
 */

#ifndef _COM_CONFIG_H
#define _COM_CONFIG_H

#include "main.h"

/* ======================== 枚举类型定义 ======================== */

/**
 * @brief 遥控器连接状态枚举
 */
typedef enum
{
    REMOTE_CONNECTED = 0,   /**< 遥控器已连接 */
    REMOTE_DISCONNECTED,    /**< 遥控器断开连接 */
} Remote_State;

/**
 * @brief 飞行状态枚举
 */
typedef enum
{
    IDLE = 0,       /**< 空闲状态 - 电机停止 */
    NORMAL,         /**< 正常飞行状态 */
    FIX_HEIGHT,     /**< 定高飞行状态 */
    FAIL,           /**< 故障状态 - 遥控器断连 */
} Flight_State;

/**
 * @brief 油门解锁状态枚举
 * @note  解锁逻辑: 油门推到最高 -> 保持1秒 -> 油门拉到最低 -> 保持1秒 -> 解锁成功
 */
typedef enum
{
    FREE = 0,       /**< 空闲状态 */
    MAX,            /**< 油门最高状态 */
    LEAVE_MAX,      /**< 离开最高状态 */
    MIN,            /**< 油门最低状态 */
    UNLOCK,         /**< 解锁成功状态 */
} Thr_state;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 遥控器数据结构体
 * @note  接收自遥控器的6通道控制数据
 */
typedef struct
{
    int16_t thr;        /**< 油门通道 (0-1000, 500为中间值) */
    int16_t yaw;        /**< 偏航通道 (0-1000, 500为中间值) */
    int16_t pit;        /**< 俯仰通道 (0-1000, 500为中间值) */
    int16_t rol;        /**< 横滚通道 (0-1000, 500为中间值) */
    uint8_t shutdown;   /**< 关机标志 (1: 关闭电源, 0: 正常运行) */
    uint8_t fix_height; /**< 定高标志 (1: 切换定高模式, 0: 不切换) */
} Remote_Data;

/**
 * @brief 陀螺仪数据结构体 (16位ADC原始值)
 */
typedef struct
{
    int16_t gyro_x;     /**< X轴角速度 (右转为正, 表示横滚角速度) */
    int16_t gyro_y;     /**< Y轴角速度 (前转为正, 表示俯仰角速度) */
    int16_t gyro_z;     /**< Z轴角速度 (顺时针为正, 表示偏航角速度) */
} Gyro_struct;

/**
 * @brief 加速度计数据结构体 (16位ADC原始值)
 */
typedef struct
{
    int16_t accel_x;    /**< X轴加速度 (前向为正) */
    int16_t accel_y;    /**< Y轴加速度 (左向为正) */
    int16_t accel_z;    /**< Z轴加速度 (上向为正, 静止时约为16384) */
} Accel_struct;

/**
 * @brief 陀螺仪+加速度计组合数据结构体
 */
typedef struct
{
    Gyro_struct gyro;   /**< 陀螺仪数据 */
    Accel_struct accel; /**< 加速度计数据 */
} Gyro_Accel_Struct;

/**
 * @brief 欧拉角结构体 (姿态解算结果)
 */
typedef struct
{
    float yaw;          /**< 偏航角 (度) */
    float pitch;        /**< 俯仰角 (度) */
    float roll;         /**< 横滚角 (度) */
} Euler_struct;

#endif /* _COM_CONFIG_H */
