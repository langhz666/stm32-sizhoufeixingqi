/**
 * @file    Int_VL53L1X.c
 * @brief   VL53L1X激光测距传感器驱动实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过I2C2接口读取激光测距数据
 *          用于定高控制功能
 */

#include "Int_VL53L1X.h"

/** @brief VL53L1X I2C设备地址 */
uint16_t dev = 0x52;

/**
 * @brief 初始化激光测距传感器
 * @note  初始化流程:
 *        1. 硬件复位 (XSHUT引脚)
 *        2. 读取设备ID验证通信
 *        3. 等待芯片启动完成
 *        4. 传感器初始化 (默认配置)
 *        5. 配置测距参数
 *        6. 启动测距
 */
void Int_VL53L1X_Init(void)
{
    uint8_t byteData = 0;
    uint16_t wordData = 0;
    uint8_t sensorState = 0;

    /* 1. 硬件复位 - 通过XSHUT引脚 */
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_RESET);
    vTaskDelay(2);
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_SET);
    vTaskDelay(2);

    /* 2. 读取设备ID，验证I2C通信 */
    VL53L1_RdByte(dev, 0x010F, &byteData);
    printf("VL53L1X Model_ID: %X\n", byteData);
    VL53L1_RdByte(dev, 0x0110, &byteData);
    printf("VL53L1X Module_Type: %X\n", byteData);
    VL53L1_RdWord(dev, 0x010F, &wordData);
    printf("VL53L1X: %X\n", wordData);

    /* 3. 等待芯片启动完成 */
    while (sensorState == 0)
    {
        VL53L1X_BootState(dev, &sensorState);
        vTaskDelay(2);
    }
    printf("Chip booted\n");

    /* 4. 传感器初始化 (默认配置) */
    VL53L1X_SensorInit(dev);

    /* 5. 配置测距参数 */
    VL53L1X_SetDistanceMode(dev, 2);          /* 测距模式: 1=短距离, 2=长距离 */
    VL53L1X_SetTimingBudgetInMs(dev, 20);     /* 测量时间: [20, 50, 100, 200, 500]ms */
    VL53L1X_SetInterMeasurementInMs(dev, 20); /* 测量间隔: 必须 >= 测量时间 */

    printf("VL53L1X Ultra Lite Driver Example running ...\n");

    /* 6. 启动测距 */
    VL53L1X_StartRanging(dev);
}

/**
 * @brief 获取激光测距值
 * @return 距离值 (mm)
 * @note   调用VL53L1X库函数获取测量结果
 */
uint16_t Int_VL53L1X_GetDistance(void)
{
    uint16_t Distance = 0;
    VL53L1X_GetDistance(dev, &Distance);
    return Distance;
}
