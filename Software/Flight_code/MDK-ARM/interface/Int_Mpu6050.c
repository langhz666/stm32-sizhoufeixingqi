/**
 * @file    Int_Mpu6050.c
 * @brief   MPU6050六轴传感器驱动实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过I2C接口读取陀螺仪和加速度计数据
 *          包含自动偏移校准功能
 */

#include "Int_mpu6050.h"

/* ======================== 偏移校准值 ======================== */

/** @brief 加速度计三轴偏移量 */
int32_t acc_x_offset = 0;
int32_t acc_y_offset = 0;
int32_t acc_z_offset = 0;

/** @brief 陀螺仪三轴偏移量 */
int32_t gyro_x_offset = 0;
int32_t gyro_y_offset = 0;
int32_t gyro_z_offset = 0;

/* ======================== 底层I2C读写函数 ======================== */

/**
 * @brief 写MPU6050寄存器
 * @param reg  寄存器地址
 * @param data 要写入的值
 */
void Int_MPU6050_Write_Reg(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR_WRITE, reg,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

/**
 * @brief 读MPU6050寄存器
 * @param reg  寄存器地址
 * @param data 读取数据存放地址
 */
void Int_MPU6050_Read_Reg(uint8_t reg, uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, reg,
                     I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

/* ======================== 偏移校准 ======================== */

/**
 * @brief 计算MPU6050偏移量
 * @note  校准流程:
 *        1. 等待飞机平稳 (连续100次采样变化量<400)
 *        2. 采样100次取平均作为偏移值
 *        3. Z轴加速度偏移基于1g (16384) 计算
 */
void Int_MPU6050_calculate_offset(void)
{
    Accel_struct current_accel = {0};
    Accel_struct last_accel = {0};
    uint8_t count = 0;

    /* 读取初始加速度值 */
    Int_MPU6050_Get_Acc(&last_accel);

    /* 1. 等待飞机平稳 - 连续100次采样变化量小于阈值 */
    while (count < 100)
    {
        Int_MPU6050_Get_Acc(&current_accel);

        /* 判断飞机是否平稳 (各轴变化量<400) */
        if (abs(current_accel.accel_x - last_accel.accel_x) < 400 &&
            abs(current_accel.accel_y - last_accel.accel_y) < 400 &&
            abs(current_accel.accel_z - last_accel.accel_z) < 400)
        {
            count++;
        }
        else
        {
            count = 0; /* 不平稳则重新计数 */
        }
        last_accel = current_accel;
        vTaskDelay(6);
    }

    /* 2. 飞机已平稳，开始偏移校准 */
    Gyro_Accel_Struct gyro_accel_data = {0};
    int32_t acc_x_sum = 0;
    int32_t acc_y_sum = 0;
    int32_t acc_z_sum = 0;
    int32_t gyro_x_sum = 0;
    int32_t gyro_y_sum = 0;
    int32_t gyro_z_sum = 0;

    for (uint8_t i = 0; i < 100; i++)
    {
        Int_MPU6050_Get_Data(&gyro_accel_data);

        /* 加速度计偏移 (Z轴基于1g = 16384) */
        acc_x_sum += (gyro_accel_data.accel.accel_x - 0);
        acc_y_sum += (gyro_accel_data.accel.accel_y - 0);
        acc_z_sum += (gyro_accel_data.accel.accel_z - 16384);

        /* 陀螺仪偏移 (静止时应为0) */
        gyro_x_sum += (gyro_accel_data.gyro.gyro_x - 0);
        gyro_y_sum += (gyro_accel_data.gyro.gyro_y - 0);
        gyro_z_sum += (gyro_accel_data.gyro.gyro_z - 0);

        vTaskDelay(6);
    }

    /* 计算平均偏移值 */
    acc_x_offset = acc_x_sum / 100;
    acc_y_offset = acc_y_sum / 100;
    acc_z_offset = acc_z_sum / 100;
    gyro_x_offset = gyro_x_sum / 100;
    gyro_y_offset = gyro_y_sum / 100;
    gyro_z_offset = gyro_z_sum / 100;
}

/* ======================== 初始化 ======================== */

/**
 * @brief 初始化MPU6050芯片
 * @note  初始化步骤:
 *        1. 复位芯片
 *        2. 设置陀螺仪量程 ±2000°/s
 *        3. 设置加速度计量程 ±2g
 *        4. 关闭中断
 *        5. 设置采样率 500Hz
 *        6. 设置低通滤波 184Hz
 *        7. 选择时钟源 (PLL)
 *        8. 执行偏移校准
 */
void Int_MPU6050_Init(void)
{
    uint8_t data = 0;

    /* 1. 复位芯片 */
    Int_MPU6050_Write_Reg(0x6B, 0x80);
    while (data != 0x40) /* 等待复位完成 (进入睡眠模式) */
    {
        Int_MPU6050_Read_Reg(0x6B, &data);
    }
    Int_MPU6050_Write_Reg(0x6B, 0x00); /* 唤醒芯片 */

    /* 2. 设置陀螺仪量程 ±2000°/s */
    Int_MPU6050_Write_Reg(0x1B, 3 << 3);

    /* 3. 设置加速度计量程 ±2g */
    Int_MPU6050_Write_Reg(0x1C, 0x00);

    /* 4. 关闭中断 */
    Int_MPU6050_Write_Reg(0x38, 0x00);

    /* 5. 关闭FIFO和I2C主机模式 */
    Int_MPU6050_Write_Reg(0x6A, 0x00);

    /* 6. 设置采样率分频 (1000Hz / (1+1) = 500Hz) */
    Int_MPU6050_Write_Reg(0x19, 0x01);

    /* 7. 设置低通滤波带宽 184Hz */
    Int_MPU6050_Write_Reg(0x1A, 1);

    /* 8. 选择时钟源 (PLL with X axis gyroscope) */
    Int_MPU6050_Write_Reg(0x6B, 0x01);

    /* 9. 使能加速度计和陀螺仪 */
    Int_MPU6050_Write_Reg(0x6C, 0x00);

    /* 10. 执行偏移校准 */
    Int_MPU6050_calculate_offset();
}

/* ======================== 数据读取 ======================== */

/**
 * @brief 获取陀螺仪数据 (已减去偏移值)
 * @param gyro 陀螺仪数据结构体指针
 * @note  寄存器地址从0x43开始，高位在前，XYZ顺序
 */
void Int_MPU6050_Get_Gyro(Gyro_struct *gyro)
{
    uint8_t hight = 0;
    uint8_t low = 0;

    /* X轴 */
    Int_MPU6050_Read_Reg(MPU_GYRO_XOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_XOUTL_REG, &low);
    gyro->gyro_x = (hight << 8 | low) - gyro_x_offset;

    /* Y轴 */
    Int_MPU6050_Read_Reg(MPU_GYRO_YOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_YOUTL_REG, &low);
    gyro->gyro_y = (hight << 8 | low) - gyro_y_offset;

    /* Z轴 */
    Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTL_REG, &low);
    gyro->gyro_z = (hight << 8 | low) - gyro_z_offset;
}

/**
 * @brief 获取加速度计数据 (已减去偏移值)
 * @param acc 加速度计数据结构体指针
 * @note  Z轴偏移基于1g (16384) 计算
 */
void Int_MPU6050_Get_Acc(Accel_struct *acc)
{
    uint8_t hight = 0;
    uint8_t low = 0;

    /* X轴 */
    Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTL_REG, &low);
    acc->accel_x = (hight << 8 | low) - acc_x_offset;

    /* Y轴 */
    Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTL_REG, &low);
    acc->accel_y = (hight << 8 | low) - acc_y_offset;

    /* Z轴 */
    Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTL_REG, &low);
    acc->accel_z = (hight << 8 | low) - acc_z_offset;
}

/**
 * @brief 获取所有传感器数据
 * @param data 传感器数据结构体指针
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data)
{
    Int_MPU6050_Get_Gyro(&data->gyro);
    Int_MPU6050_Get_Acc(&data->accel);
}
