#ifndef _COM_CONFIG_H
#define _COM_CONFIG_H

#include "main.h"

// 连接状态
typedef enum
{
    REMOTE_CONNECTED = 0,
    REMOTE_DISCONNECTED,
} Remote_State;

// 飞行状态
typedef enum
{
    IDLE = 0,
    NORMAL,
    FIX_HEIGHT,
    FAIL,
} Flight_State;

// 油门解锁状态
typedef enum
{
    FREE = 0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
} Thr_state;

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown;   // 1: 关闭  0: 不关机
    uint8_t fix_height; // 1. 切换定高和不定高 0: 不切换
} Remote_Data;

// 陀螺仪数据  16位ADC的值
typedef struct
{
    int16_t gyro_x; // 往右飞为正   表示横滚角
    int16_t gyro_y; // 向前飞转动为正 表示俯仰角
    int16_t gyro_z; // 逆时针转动为正  表示偏航角
} Gyro_struct;
// 16位ADC的值
typedef struct
{
    int16_t accel_x; // 往前为正
    int16_t accel_y; // 往左为正
    int16_t accel_z; // 朝上的加速度为正
} Accel_struct;

typedef struct
{
    Gyro_struct gyro;
    Accel_struct accel;
} Gyro_Accel_Struct;

// 解算得到的欧拉角
typedef struct
{
    float yaw;
    float pitch;
    float roll;
} Euler_struct;

#endif // !_COM_CONFIG_H
