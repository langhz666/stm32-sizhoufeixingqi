/**
 * @file    Int_VL53L1X.h
 * @brief   VL53L1X激光测距传感器驱动 - 定高控制
 * @author  langhz666
 * @date    2025-09-27
 * @note    VL53L1X是一款ToF激光测距传感器
 *          通过I2C2接口与STM32通信
 *          测量范围: 40mm ~ 4000mm
 */

#ifndef __INT_VL53L1X__
#define __INT_VL53L1X__

#include "vl53l1_platform.h"
#include "VL53L1X_api.h"
#include "VL53L1X_calibration.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 初始化激光测距传感器
 * @note  包括硬件复位、传感器初始化、配置测距参数
 *        测距模式: 长距离模式
 *        测量周期: 20ms
 */
void Int_VL53L1X_Init(void);

/**
 * @brief 获取激光测距值
 * @return 距离值 (mm)
 * @note   返回当前测量的距离值
 */
uint16_t Int_VL53L1X_GetDistance(void);

#endif /* __INT_VL53L1X__ */
