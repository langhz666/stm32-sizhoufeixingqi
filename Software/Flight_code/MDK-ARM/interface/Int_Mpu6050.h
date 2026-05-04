/**
 * @file    Int_Mpu6050.h
 * @brief   MPU6050六轴传感器驱动 - I2C通信接口
 * @author  langhz666
 * @date    2025-09-27
 * @note    MPU6050包含三轴陀螺仪和三轴加速度计
 *          通过I2C接口与STM32通信
 */

#ifndef __INT_MPU6050__
#define __INT_MPU6050__

#include "i2c.h"
#include "Com_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"

/* ======================== MPU6050地址定义 ======================== */

#define MPU6050_ADDR        0x68    /**< MPU6050设备地址 */
#define MPU6050_ADDR_WRITE  0xD0    /**< 写地址 (ADDR << 1 | 0) */
#define MPU6050_ADDR_READ   0xD1    /**< 读地址 (ADDR << 1 | 1) */

/* ======================== MPU6050寄存器地址 ======================== */

/* 自检寄存器 */
#define MPU_SELF_TESTX_REG  0X0D    /**< X轴自检寄存器 */
#define MPU_SELF_TESTY_REG  0X0E    /**< Y轴自检寄存器 */
#define MPU_SELF_TESTZ_REG  0X0F    /**< Z轴自检寄存器 */
#define MPU_SELF_TESTA_REG  0X10    /**< 加速度自检寄存器 */

/* 配置寄存器 */
#define MPU_SAMPLE_RATE_REG 0X19    /**< 采样频率分频器 */
#define MPU_CFG_REG         0X1A    /**< 配置寄存器 */
#define MPU_GYRO_CFG_REG    0X1B    /**< 陀螺仪配置寄存器 */
#define MPU_ACCEL_CFG_REG   0X1C    /**< 加速度计配置寄存器 */
#define MPU_MOTION_DET_REG  0X1F    /**< 运动检测阈值配置寄存器 */

/* FIFO寄存器 */
#define MPU_FIFO_EN_REG     0X23    /**< FIFO使能寄存器 */

/* I2C主从机控制寄存器 */
#define MPU_I2CMST_CTRL_REG    0X24 /**< I2C主机控制寄存器 */
#define MPU_I2CSLV0_ADDR_REG   0X25 /**< I2C从机0地址寄存器 */
#define MPU_I2CSLV0_REG        0X26 /**< I2C从机0数据地址寄存器 */
#define MPU_I2CSLV0_CTRL_REG   0X27 /**< I2C从机0控制寄存器 */
#define MPU_I2CSLV1_ADDR_REG   0X28 /**< I2C从机1地址寄存器 */
#define MPU_I2CSLV1_REG        0X29 /**< I2C从机1数据地址寄存器 */
#define MPU_I2CSLV1_CTRL_REG   0X2A /**< I2C从机1控制寄存器 */
#define MPU_I2CSLV2_ADDR_REG   0X2B /**< I2C从机2地址寄存器 */
#define MPU_I2CSLV2_REG        0X2C /**< I2C从机2数据地址寄存器 */
#define MPU_I2CSLV2_CTRL_REG   0X2D /**< I2C从机2控制寄存器 */
#define MPU_I2CSLV3_ADDR_REG   0X2E /**< I2C从机3地址寄存器 */
#define MPU_I2CSLV3_REG        0X2F /**< I2C从机3数据地址寄存器 */
#define MPU_I2CSLV3_CTRL_REG   0X30 /**< I2C从机3控制寄存器 */
#define MPU_I2CSLV4_ADDR_REG   0X31 /**< I2C从机4地址寄存器 */
#define MPU_I2CSLV4_REG        0X32 /**< I2C从机4数据地址寄存器 */
#define MPU_I2CSLV4_DO_REG     0X33 /**< I2C从机4写数据寄存器 */
#define MPU_I2CSLV4_CTRL_REG   0X34 /**< I2C从机4控制寄存器 */
#define MPU_I2CSLV4_DI_REG     0X35 /**< I2C从机4读数据寄存器 */

/* 中断和状态寄存器 */
#define MPU_I2CMST_STA_REG  0X36    /**< I2C主机状态寄存器 */
#define MPU_INTBP_CFG_REG   0X37    /**< 中断/旁路配置寄存器 */
#define MPU_INT_EN_REG      0X38    /**< 中断使能寄存器 */
#define MPU_INT_STA_REG     0X3A    /**< 中断状态寄存器 */

/* 加速度计数据寄存器 (高位在前) */
#define MPU_ACCEL_XOUTH_REG 0X3B    /**< X轴加速度高8位 */
#define MPU_ACCEL_XOUTL_REG 0X3C    /**< X轴加速度低8位 */
#define MPU_ACCEL_YOUTH_REG 0X3D    /**< Y轴加速度高8位 */
#define MPU_ACCEL_YOUTL_REG 0X3E    /**< Y轴加速度低8位 */
#define MPU_ACCEL_ZOUTH_REG 0X3F    /**< Z轴加速度高8位 */
#define MPU_ACCEL_ZOUTL_REG 0X40    /**< Z轴加速度低8位 */

/* 温度数据寄存器 */
#define MPU_TEMP_OUTH_REG   0X41    /**< 温度高8位 */
#define MPU_TEMP_OUTL_REG   0X42    /**< 温度低8位 */

/* 陀螺仪数据寄存器 (高位在前) */
#define MPU_GYRO_XOUTH_REG  0X43    /**< X轴角速度高8位 */
#define MPU_GYRO_XOUTL_REG  0X44    /**< X轴角速度低8位 */
#define MPU_GYRO_YOUTH_REG  0X45    /**< Y轴角速度高8位 */
#define MPU_GYRO_YOUTL_REG  0X46    /**< Y轴角速度低8位 */
#define MPU_GYRO_ZOUTH_REG  0X47    /**< Z轴角速度高8位 */
#define MPU_GYRO_ZOUTL_REG  0X48    /**< Z轴角速度低8位 */

/* I2C从机数据输出寄存器 */
#define MPU_I2CSLV0_DO_REG  0X63    /**< I2C从机0数据寄存器 */
#define MPU_I2CSLV1_DO_REG  0X64    /**< I2C从机1数据寄存器 */
#define MPU_I2CSLV2_DO_REG  0X65    /**< I2C从机2数据寄存器 */
#define MPU_I2CSLV3_DO_REG  0X66    /**< I2C从机3数据寄存器 */

/* 其他控制寄存器 */
#define MPU_I2CMST_DELAY_REG  0X67  /**< I2C主机延时配置寄存器 */
#define MPU_SIGPATH_RST_REG   0X68  /**< 信号通路复位寄存器 */
#define MPU_MDETECT_CTRL_REG  0X69  /**< 运动检测控制寄存器 */
#define MPU_USER_CTRL_REG     0X6A  /**< 用户控制寄存器 */
#define MPU_PWR_MGMT1_REG     0X6B  /**< 电源管理寄存器1 */
#define MPU_PWR_MGMT2_REG     0X6C  /**< 电源管理寄存器2 */
#define MPU_FIFO_CNTH_REG     0X72  /**< FIFO计数高8位 */
#define MPU_FIFO_CNTL_REG     0X73  /**< FIFO计数低8位 */
#define MPU_FIFO_RW_REG       0X74  /**< FIFO读写寄存器 */
#define MPU_DEVICE_ID_REG     0X75  /**< 设备ID寄存器 */

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化MPU6050芯片
 * @note  包括复位、配置量程、采样率、低通滤波、偏移校准
 */
void Int_MPU6050_Init(void);

/**
 * @brief 获取陀螺仪数据 (已减去偏移值)
 * @param gyro 陀螺仪数据结构体指针
 */
void Int_MPU6050_Get_Gyro(Gyro_struct *gyro);

/**
 * @brief 获取加速度计数据 (已减去偏移值)
 * @param acc 加速度计数据结构体指针
 */
void Int_MPU6050_Get_Acc(Accel_struct *acc);

/**
 * @brief 获取所有传感器数据 (陀螺仪+加速度计)
 * @param data 传感器数据结构体指针
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data);

#endif /* __INT_MPU6050__ */
