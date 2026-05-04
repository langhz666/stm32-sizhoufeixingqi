/**
 * @file    Com_imu.c
 * @brief   IMU姿态解算实现 - Mahony互补滤波算法
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 算法原理详解:
 *
 * 1. 坐标系定义 (东北天坐标系):
 *    - X轴: 前向 (机头方向)
 *    - Y轴: 左向
 *    - Z轴: 上向 (垂直地面向上)
 *    - 重力向量: G = [0, 0, 1] (归一化后)
 *
 * 2. 欧拉角定义:
 *    - Pitch (俯仰角): 绕Y轴旋转, 抬头为正, 范围 [-90°, +90°]
 *    - Roll (横滚角): 绕X轴旋转, 右倾为正, 范围 [-180°, +180°]
 *    - Yaw (偏航角): 绕Z轴旋转, 顺时针为正, 范围 [-180°, +180°]
 *
 * 3. 四元数表示:
 *    q = q0 + q1*i + q2*j + q3*k
 *    其中 q0 为实部, q1/q2/q3 为虚部
 *    单位四元数满足: q0² + q1² + q2² + q3² = 1
 *
 * 4. Mahony互补滤波原理:
 *    - 陀螺仪: 短期精度高, 长期有漂移
 *    - 加速度计: 长期稳定, 短期有噪声
 *    - 通过PI控制器融合两者优点:
 *      Gyro_corrected = Gyro_raw + Kp * error + Ki * ∫error dt
 *    - error = Acc × Gravity (叉积表示姿态误差)
 *
 * 5. 四元数微分方程:
 *    dq/dt = 0.5 * q ⊗ ω
 *    其中 ω 为角速度向量, ⊗ 为四元数乘法
 *
 * 6. 一阶龙格库塔积分:
 *    q(t+dt) = q(t) + dq/dt * dt
 *
 * @note    调参建议:
 *          - Kp: 加速度计修正权重, 增大则更信任加速度计, 响应更快但噪声更大
 *          - Ki: 陀螺仪零偏补偿, 增大则消除漂移更快, 但可能导致超调
 *          - 默认值 Kp=0.8, Ki=0.0003 适用于大多数四旋翼
 */

#include "Com_IMU.h"

/* ======================== 常量定义 ======================== */

/**
 * @brief 弧度转角度系数
 * @note  180 / π ≈ 57.2957795
 *        用于将弧度制的欧拉角转换为角度制
 */
float RtA = 57.2957795f;

/**
 * @brief 陀螺仪原始值转度/秒系数
 * @note  量程 ±2000°/s, 16位ADC (±32768)
 *        转换系数 = 2000 / 32768 = 4000 / 65536
 *        例: 原始值 16384 对应 1000°/s
 */
float Gyro_G = 4000.0 / 65536;

/**
 * @brief 陀螺仪原始值转弧度/秒系数
 * @note  转换系数 = Gyro_G * π / 180
 *        用于四元数积分计算 (需要弧度单位)
 */
float Gyro_Gr = 4000.0 / 65536 / 180 * 3.1415926;

/** @brief 平方宏定义 */
#define squa(Sq) (((float)Sq) * ((float)Sq))

/**
 * @brief 快速平方根倒数算法 (1/sqrt(x))
 * @param number 输入值
 * @return 1/sqrt(number)
 *
 * @note    算法来源: Quake III Arena 游戏引擎
 *          原理: 利用IEEE 754浮点数的位表示进行近似计算
 *
 *          步骤:
 *          1. 将浮点数的位模式解释为整数
 *          2. 对整数进行右移和减法 (近似对数域的除法)
 *          3. 将结果解释回浮点数
 *          4. 进行一次牛顿迭代提高精度
 *
 *          优势: 比标准库 sqrt() 快 3-4 倍
 *          精度: 误差 < 1% (一次牛顿迭代后)
 *
 *          数学原理:
 *          float 的位表示: value = 2^e * m
 *          log2(value) ≈ e + m
 *          1/sqrt(value) = 2^(-e/2) * (1/sqrt(m))
 *          通过整数运算近似实现
 */
static float Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;

    /* 关键步骤: 将浮点数位模式解释为整数 */
    i = *(long *)&y;

    /* 魔数 0x5f3759df: 近似计算 1/sqrt(x) 的初始值 */
    i = 0x5f3759df - (i >> 1);

    /* 将整数位模式解释回浮点数 */
    y = *(float *)&i;

    /* 一次牛顿迭代: y = y * (1.5 - x2 * y * y) */
    y = y * (threehalfs - (x2 * y * y));

    return y;
}

/**
 * @brief Z轴合成加速度 (考虑机体倾斜)
 * @note  用于定高控制，消除机体倾斜对Z轴加速度测量的影响
 *        计算公式: normAccz = Acc · Rz
 *        其中 Rz 为旋转矩阵的第三列 (Z轴方向)
 */
static float normAccz;

/**
 * @brief 姿态解算主函数 - Mahony互补滤波
 * @param gyroAccel 陀螺仪+加速度计原始数据指针
 * @param eulerAngle 输出的欧拉角指针
 * @param dt         积分时间 (秒), 本系统为 0.006s (6ms)
 *
 * @note    详细算法流程:
 *
 *          Step 1: 从四元数计算重力分量
 *          ================================
 *          旋转矩阵 R 的第三列 (重力方向) 可以从四元数计算:
 *          R[2][0] = 2*(q1*q3 - q0*q2)
 *          R[2][1] = 2*(q0*q1 + q2*q3)
 *          R[2][2] = 1 - 2*(q1² + q2²)
 *
 *          Step 2: 加速度计归一化
 *          ================================
 *          将加速度计数据归一化为单位向量:
 *          Acc_norm = Acc / |Acc|
 *          使用快速平方根倒数算法加速计算
 *
 *          Step 3: 计算姿态误差 (叉积)
 *          ================================
 *          误差 = Acc_norm × Gravity_estimated
 *          叉积表示两个向量之间的旋转误差
 *          当误差为0时，表示估计的重力方向与测量的加速度方向一致
 *
 *          Step 4: PI控制器修正陀螺仪
 *          ================================
 *          Gyro_corrected = Gyro_raw + Kp * error + Ki * ∫error dt
 *          - Kp 项: 快速响应姿态误差
 *          - Ki 项: 消除陀螺仪零偏漂移
 *
 *          Step 5: 四元数微分方程
 *          ================================
 *          dq/dt = 0.5 * q ⊗ ω
 *          展开为矩阵形式:
 *          [dq0/dt]   [ 0  -ωx  -ωy  -ωz] [q0]
 *          [dq1/dt] = [ ωx  0   ωz  -ωy] [q1]
 *          [dq2/dt]   [ ωy -ωz  0    ωx] [q2]
 *          [dq3/dt]   [ ωz  ωy -ωx   0 ] [q3]
 *
 *          Step 6: 一阶龙格库塔积分
 *          ================================
 *          q(t+dt) = q(t) + dq/dt * dt
 *          简单有效，适合嵌入式实时计算
 *
 *          Step 7: 四元数归一化
 *          ================================
 *          保证单位四元数约束: |q| = 1
 *          防止数值误差累积导致姿态发散
 *
 *          Step 8: 四元数转欧拉角
 *          ================================
 *          从旋转矩阵元素反推欧拉角:
 *          Pitch = asin(R[2][0]) = asin(2*(q0*q2 - q1*q3))
 *          Roll  = atan2(R[2][1], R[2][2])
 *          Yaw   += ωz * dt (陀螺仪积分, 加速度计无法观测偏航)
 *
 *          Step 9: 计算Z轴合成加速度
 *          ================================
 *          normAccz = Acc · Rz
 *          消除机体倾斜影响，用于定高控制
 */
void Common_IMU_GetEulerAngle(Gyro_Accel_Struct *gyroAccel,
                              Euler_struct *eulerAngle,
                              float dt)
{
    /* 临时向量结构体 */
    volatile struct V
    {
        float x;
        float y;
        float z;
    } Gravity, Acc, Gyro, AccGravity;

    /* 静态变量: 保持状态跨函数调用 */
    static struct V GyroIntegError = {0};   /* 陀螺仪积分误差累积 */
    static float KpDef = 0.8f;              /* 比例增益 - 加速度计修正权重 */
    static float KiDef = 0.0003f;           /* 积分增益 - 陀螺仪漂移补偿 */
    static Quaternion_Struct NumQ = {1, 0, 0, 0}; /* 姿态四元数, 初始为单位四元数 */

    float q0_t, q1_t, q2_t, q3_t;
    float NormQuat;
    float HalfTime = dt * 0.5f; /* 预计算 dt/2, 减少运行时乘法 */

    /* ==================== Step 1: 计算重力分量 ==================== */
    /*
     * 从四元数计算旋转矩阵的第三列 (重力方向在机体坐标系的投影)
     *
     * 旋转矩阵 R (四元数表示):
     * R = [1-2(q2²+q3²)  2(q1q2-q0q3)  2(q1q3+q0q2)]
     *     [2(q1q2+q0q3)  1-2(q1²+q3²)  2(q2q3-q0q1)]
     *     [2(q1q3-q0q2)  2(q2q3+q0q1)  1-2(q1²+q2²)]
     *
     * 重力向量 G = [0, 0, 1] 在机体系的投影 = R^T * G = R的第三列
     */
    Gravity.x = 2 * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
    Gravity.y = 2 * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
    Gravity.z = 1 - 2 * (NumQ.q1 * NumQ.q1 + NumQ.q2 * NumQ.q2);

    /* ==================== Step 2: 加速度计归一化 ==================== */
    /*
     * 将加速度计原始数据转换为单位向量
     * 使用快速平方根倒数算法: 1/|a| = Q_rsqrt(ax² + ay² + az²)
     * 然后: ax_norm = ax * (1/|a|)
     */
    NormQuat = Q_rsqrt(squa(gyroAccel->accel.accel_x) +
                       squa(gyroAccel->accel.accel_y) +
                       squa(gyroAccel->accel.accel_z));
    Acc.x = gyroAccel->accel.accel_x * NormQuat;
    Acc.y = gyroAccel->accel.accel_y * NormQuat;
    Acc.z = gyroAccel->accel.accel_z * NormQuat;

    /* ==================== Step 3: 计算姿态误差 (叉积) ==================== */
    /*
     * 误差 = Acc_norm × Gravity_estimated
     *
     * 叉积的物理意义:
     * - 当两个向量平行时, 叉积为0 (无误差)
     * - 当两个向量有夹角时, 叉积方向为旋转轴, 大小与夹角成正比
     *
     * 这个误差用于修正陀螺仪的漂移
     */
    AccGravity.x = (Acc.y * Gravity.z - Acc.z * Gravity.y);
    AccGravity.y = (Acc.z * Gravity.x - Acc.x * Gravity.z);
    AccGravity.z = (Acc.x * Gravity.y - Acc.y * Gravity.x);

    /* ==================== Step 4: 积分累加误差 ==================== */
    /*
     * Ki积分项: 用于消除陀螺仪的零偏漂移
     * 累积误差 = Σ(误差 * Ki * dt)
     *
     * 为什么需要积分项:
     * - 陀螺仪存在零偏 (bias), 即静止时输出不为0
     * - 零偏会导致姿态角持续漂移
     * - 积分项可以估计并补偿这个零偏
     */
    GyroIntegError.x += AccGravity.x * KiDef;
    GyroIntegError.y += AccGravity.y * KiDef;
    GyroIntegError.z += AccGravity.z * KiDef;

    /* ==================== Step 5: 陀螺仪数据融合 ==================== */
    /*
     * 融合公式: Gyro_corrected = Gyro_raw + Kp*error + Ki*∫error
     *
     * 各项作用:
     * - Gyro_raw: 陀螺仪原始角速度 (短期精度高)
     * - Kp*error: 比例修正 (快速响应姿态误差)
     * - Ki*∫error: 积分修正 (消除零偏漂移)
     *
     * 单位转换:
     * - gyro_x/y/z 原始值单位为 ADC值
     * - 乘以 Gyro_Gr 转换为 弧度/秒
     */
    Gyro.x = gyroAccel->gyro.gyro_x * Gyro_Gr + KpDef * AccGravity.x + GyroIntegError.x;
    Gyro.y = gyroAccel->gyro.gyro_y * Gyro_Gr + KpDef * AccGravity.y + GyroIntegError.y;
    Gyro.z = gyroAccel->gyro.gyro_z * Gyro_Gr + KpDef * AccGravity.z + GyroIntegError.z;

    /* ==================== Step 6: 四元数微分方程 (一阶龙格库塔) ==================== */
    /*
     * 四元数微分方程: dq/dt = 0.5 * q ⊗ ω
     *
     * 展开为分量形式:
     * dq0/dt = 0.5 * (-q1*ωx - q2*ωy - q3*ωz)
     * dq1/dt = 0.5 * ( q0*ωx - q3*ωy + q2*ωz)
     * dq2/dt = 0.5 * ( q3*ωx + q0*ωy - q1*ωz)
     * dq3/dt = 0.5 * (-q2*ωx + q1*ωy + q0*ωz)
     *
     * 一阶龙格库塔积分: q(t+dt) = q(t) + dq/dt * dt
     * HalfTime = dt/2 预计算, 减少运行时乘法次数
     */
    q0_t = (-NumQ.q1 * Gyro.x - NumQ.q2 * Gyro.y - NumQ.q3 * Gyro.z) * HalfTime;
    q1_t = (NumQ.q0 * Gyro.x - NumQ.q3 * Gyro.y + NumQ.q2 * Gyro.z) * HalfTime;
    q2_t = (NumQ.q3 * Gyro.x + NumQ.q0 * Gyro.y - NumQ.q1 * Gyro.z) * HalfTime;
    q3_t = (-NumQ.q2 * Gyro.x + NumQ.q1 * Gyro.y + NumQ.q0 * Gyro.z) * HalfTime;

    /* 更新四元数 */
    NumQ.q0 += q0_t;
    NumQ.q1 += q1_t;
    NumQ.q2 += q2_t;
    NumQ.q3 += q3_t;

    /* ==================== Step 7: 四元数归一化 ==================== */
    /*
     * 保证单位四元数约束: |q| = q0² + q1² + q2² + q3² = 1
     *
     * 为什么需要归一化:
     * - 数值积分会引入误差, 导致 |q| 逐渐偏离1
     * - 非单位四元数会导致姿态表示错误
     * - 每次更新后必须归一化
     *
     * 使用快速平方根倒数算法: q_norm = q * (1/|q|)
     */
    NormQuat = Q_rsqrt(squa(NumQ.q0) + squa(NumQ.q1) + squa(NumQ.q2) + squa(NumQ.q3));
    NumQ.q0 *= NormQuat;
    NumQ.q1 *= NormQuat;
    NumQ.q2 *= NormQuat;
    NumQ.q3 *= NormQuat;

    /* ==================== Step 8: 四元数转欧拉角 ==================== */
    /*
     * 从旋转矩阵元素反推欧拉角 (ZYX顺序):
     *
     * 旋转矩阵 R (四元数表示):
     * R[0][0] = 1-2(q2²+q3²)    R[0][1] = 2(q1q2-q0q3)    R[0][2] = 2(q1q3+q0q2)
     * R[1][0] = 2(q1q2+q0q3)    R[1][1] = 1-2(q1²+q3²)    R[1][2] = 2(q2q3-q0q1)
     * R[2][0] = 2(q1q3-q0q2)    R[2][1] = 2(q2q3+q0q1)    R[2][2] = 1-2(q1²+q2²)
     *
     * 欧拉角计算:
     * Pitch = asin(R[2][0]) = asin(2*(q0*q2 - q1*q3))
     * Roll  = atan2(R[2][1], R[2][2])
     * Yaw   = atan2(R[1][0], R[0][0]) (本系统使用陀螺仪积分)
     */
    float vecxZ = 2 * NumQ.q0 * NumQ.q2 - 2 * NumQ.q1 * NumQ.q3;     /* R[2][0] */
    float vecyZ = 2 * NumQ.q2 * NumQ.q3 + 2 * NumQ.q0 * NumQ.q1;     /* R[2][1] */
    float veczZ = 1 - 2 * NumQ.q1 * NumQ.q1 - 2 * NumQ.q2 * NumQ.q2; /* R[2][2] */

    /* ==================== Step 9: 偏航角积分 ==================== */
    /*
     * 偏航角 (Yaw) 的特殊处理:
     * - 加速度计无法观测偏航角 (绕Z轴旋转不改变重力方向)
     * - 因此偏航角只能通过陀螺仪Z轴积分得到
     * - 会存在累积漂移, 但在飞行控制中可接受
     *
     * 死区处理:
     * - 当角速度 < 0.5°/s 时, 认为是零偏噪声, 不积分
     * - 防止静止时偏航角缓慢漂移
     */
    float yaw_G = gyroAccel->gyro.gyro_z * Gyro_G; /* 转换为 °/s */
    if ((yaw_G > 0.5f) || (yaw_G < -0.5))
    {
        eulerAngle->yaw += yaw_G * dt; /* 积分: 角度 += 角速度 * 时间 */
    }

    /* ==================== Step 10: 计算俯仰角和横滚角 ==================== */
    /*
     * 从旋转矩阵元素反推:
     * Pitch = asin(R[2][0]) * (180/π)
     * Roll  = atan2(R[2][1], R[2][2]) * (180/π)
     *
     * 注意: asin 返回值范围 [-π/2, π/2], 即 [-90°, 90°]
     *       atan2 返回值范围 [-π, π], 即 [-180°, 180°]
     */
    eulerAngle->pitch = asin(vecxZ) * RtA;
    eulerAngle->roll = atan2f(vecyZ, veczZ) * RtA;

    /* ==================== Step 11: 计算Z轴合成加速度 ==================== */
    /*
     * 公式: normAccz = Acc · Rz
     * 其中 Rz = [vecxZ, vecyZ, veczZ] 是旋转矩阵的第三列
     *
     * 物理意义:
     * - 将机体坐标系的加速度投影到世界坐标系的Z轴
     * - 消除机体倾斜对Z轴加速度测量的影响
     * - 静止时 normAccz ≈ 1g (9.8 m/s²)
     * - 用于定高控制中的加速度补偿
     */
    normAccz = gyroAccel->accel.accel_x * vecxZ
             + gyroAccel->accel.accel_y * vecyZ
             + gyroAccel->accel.accel_z * veczZ;
}

/**
 * @brief 获取Z轴合成加速度
 * @return Z轴加速度值 (原始ADC值单位)
 * @note   用于定高控制，消除机体倾斜影响
 *         静止时约为 16384 (对应1g, ±2g量程)
 */
float Common_IMU_GetNormAccZ(void)
{
    return normAccz;
}
