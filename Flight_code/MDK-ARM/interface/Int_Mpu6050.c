#include "Int_mpu6050.h"

// 保存偏移量的值
int32_t acc_x_offset = 0;
int32_t acc_y_offset = 0;
int32_t acc_z_offset = 0;

int32_t gyro_x_offset = 0;
int32_t gyro_y_offset = 0;
int32_t gyro_z_offset = 0;

/**
 * @brief 写寄存器
 *
 * @param reg 寄存器地址
 * @param data 寄存器的值
 */
void Int_MPU6050_Write_Reg(uint8_t reg, uint8_t data)
{
    // HAL有固定的I2C读写函数
    // 1. 句柄(hi2c1) 2. 从设备地址(0x68) 3. 寄存器地址 reg 4. 寄存器地址的位数 5. 写入的数据地址 6. 写入的字节个数 7. 超时时间
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR_WRITE, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

void Int_MPU6050_Read_Reg(uint8_t reg, uint8_t *data)
{
    // 1. 句柄(hi2c1) 2. 从设备地址(0x68) 3. 寄存器地址 reg 4. 寄存器地址的位数 5. 存放读取数据的地址 6. 读的字节个数 7. 超时时间
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR_READ, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

/**
 * @brief 在初始化MPU6050完成之后 对MPU6050进行零偏校准
 *
 */
void Int_MPU6050_calculate_offset(void)
{
    // 1. 等待飞机停放平稳
    // 判断飞机是否停放平稳的标准: 前后两次加速度的值差值小于200 连续100次
    Accel_struct current_accel = {0};
    Accel_struct last_accel = {0};
    uint8_t count = 0;
    Int_MPU6050_Get_Acc(&last_accel);

    while (count < 100)
    {
        Int_MPU6050_Get_Acc(&current_accel);
        // 判断飞机是否平稳 选用的参数过小 会造成一直无法判断为平稳
        if (abs(current_accel.accel_x - last_accel.accel_x) < 400 && abs(current_accel.accel_y - last_accel.accel_y) < 400 && abs(current_accel.accel_z - last_accel.accel_z) < 400)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        last_accel = current_accel;
        vTaskDelay(6);
    }

    // 2. 飞机已经平稳 开始进行零偏校准
    Gyro_Accel_Struct gyro_accel_data = {0};
    int32_t acc_x_sum = 0;
    int32_t acc_y_sum = 0;
    int32_t acc_z_sum = 0;

    int32_t gyro_x_sum = 0;
    int32_t gyro_y_sum = 0;
    int32_t gyro_z_sum = 0;
    for (uint8_t i = 0; i < 100; i++)
    {
        // 重新读取加速度和角速度
        Int_MPU6050_Get_Data(&gyro_accel_data);
        acc_x_sum += (gyro_accel_data.accel.accel_x - 0);
        acc_y_sum += (gyro_accel_data.accel.accel_y - 0);
        // Z轴加速度的初始化应该就是1g  => 量程是±2g => 16384
        acc_z_sum += (gyro_accel_data.accel.accel_z - 16384);

        gyro_x_sum += (gyro_accel_data.gyro.gyro_x - 0);
        gyro_y_sum += (gyro_accel_data.gyro.gyro_y - 0);
        gyro_z_sum += (gyro_accel_data.gyro.gyro_z - 0);

        // 每次测量数据需要添加延迟  多次测量取平均值才有意义
        vTaskDelay(6);
    }

    acc_x_offset = acc_x_sum / 100;
    acc_y_offset = acc_y_sum / 100;
    acc_z_offset = acc_z_sum / 100;

    gyro_x_offset = gyro_x_sum / 100;
    gyro_y_offset = gyro_y_sum / 100;
    gyro_z_offset = gyro_z_sum / 100;
}

/**
 * @brief 初始化MPU6050芯片
 *
 */
void Int_MPU6050_Init(void)
{
    // 1. 重启芯片 重置所有寄存器的值 => 写电源管理寄存器1  => DEVICE_RESET
    Int_MPU6050_Write_Reg(0x6B, 0x80);
    uint8_t data = 0;
    // 重置完成之后 0x6B寄存器的值是0x40 表示当前为低功耗模式
    while (data != 0x40)
    {
        Int_MPU6050_Read_Reg(0x6B, &data);
    }
    // 唤醒MPU6050  进入到正常工作状态
    Int_MPU6050_Write_Reg(0x6B, 0x00);

    // 2. 选择合适的量程 => 在够用的范围内 选择的越小越好 => 精度高
    // 2.1 填写角速度量程为±2000°/s
    Int_MPU6050_Write_Reg(0x1B, 3 << 3);

    // 2.2 填写加速度量程为±2g
    Int_MPU6050_Write_Reg(0x1C, 0x00);

    // 3. 关闭中断使能  因为用不到中断
    Int_MPU6050_Write_Reg(0x38, 0x00);

    // 4. 用户配置寄存器 不使用FIFO队列  不使用扩展的I2C
    Int_MPU6050_Write_Reg(0x6A, 0x00);

    // 5. 设置采样频率 => 陀螺仪监控三轴加速度和三轴角速度 => 默认频率 1000HZ => 1ms读取一次
    // 基本逻辑 => 采样率必须大于后续数据的使用频率  否则失真 => 香农定理 采样率 >= 2倍使用频率
    // 设置采样分频为2 => 填写的值就是2-1
    Int_MPU6050_Write_Reg(0x19, 0x01);

    // 6. 设置低通滤波的值为184Hz 188Hz => 1
    Int_MPU6050_Write_Reg(0x1A, 1);

    // 7. 配置使用的系统时钟为添加PLL的
    Int_MPU6050_Write_Reg(0x6B, 0x01);

    // 8. 使能加速度传感器和角速度传感器
    Int_MPU6050_Write_Reg(0x6C, 0x00);

    // 9. 进行零偏校准
    Int_MPU6050_calculate_offset();
}

/**
 * @brief 读取三轴角速度  => 需要进行零偏校准 => 本身抖动不严重
 *
 * @param gyro
 */
void Int_MPU6050_Get_Gyro(Gyro_struct *gyro)
{
    // 存储角速度的寄存器地址从0x43开始 高8位在前  XYZ的顺序
    uint8_t hight = 0;
    uint8_t low = 0;
    // X轴
    Int_MPU6050_Read_Reg(MPU_GYRO_XOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_XOUTL_REG, &low);
    gyro->gyro_x = (hight << 8 | low) - gyro_x_offset;
    // Y轴
    Int_MPU6050_Read_Reg(MPU_GYRO_YOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_YOUTL_REG, &low);
    gyro->gyro_y = (hight << 8 | low) - gyro_y_offset;
    // Z轴
    Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_GYRO_ZOUTL_REG, &low);
    gyro->gyro_z = (hight << 8 | low) - gyro_z_offset;
}

/**
 * @brief 读取三轴加速度  抖动比较严重 需要零偏校准  Z轴值不为0
 *
 * @param acc
 */
void Int_MPU6050_Get_Acc(Accel_struct *acc)
{
    uint8_t hight = 0;
    uint8_t low = 0;
    // X轴
    Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_XOUTL_REG, &low);
    acc->accel_x = (hight << 8 | low) - acc_x_offset;
    // Y轴
    Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_YOUTL_REG, &low);
    acc->accel_y = (hight << 8 | low) - acc_y_offset;
    // Z轴
    Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTH_REG, &hight);
    Int_MPU6050_Read_Reg(MPU_ACCEL_ZOUTL_REG, &low);
    acc->accel_z = (hight << 8 | low) - acc_z_offset;
}

/**
 * @brief 获取所有的六轴数据
 *
 * @param data
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data)
{
    Int_MPU6050_Get_Gyro(&data->gyro);
    Int_MPU6050_Get_Acc(&data->accel);
}
