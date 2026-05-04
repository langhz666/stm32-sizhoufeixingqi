/**
 * @file    App_flight.c
 * @brief   飞行控制应用层实现
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 四旋翼飞行器核心控制逻辑详解:
 *
 * 1. 硬件配置:
 *    - 主控: STM32F103C8T6 (ARM Cortex-M3, 72MHz)
 *    - IMU: MPU6050 (六轴: 三轴陀螺仪 + 三轴加速度计)
 *    - 布局: X型四旋翼
 *    - 电机: 4个无刷电机, PWM调速
 *
 * 2. 电机布局 (俯视图):
 *
 *        左上(3)          右上(1)
 *           \            /
 *            \          /
 *             \        /
 *              --------
 *             /        \
 *            /          \
 *           /            \
 *        左下(4)          右下(2)
 *
 *    电机编号: 1=右上, 2=右下, 3=左上, 4=左下
 *    旋转方向: 1,3 顺时针; 2,4 逆时针 (形成X型力矩平衡)
 *
 * 3. 坐标系定义 (东北天):
 *    - X轴: 前向 (机头方向)
 *    - Y轴: 左向
 *    - Z轴: 上向 (垂直地面向上)
 *
 * 4. 控制架构:
 *
 *    遥控器指令        当前姿态
 *        ↓                ↓
 *    [目标角度] <-- [角度误差] --> [外环PID] --> [目标角速度]
 *                                            ↓
 *                                    [角速度误差]
 *                                            ↓
 *                                    [内环PID] --> [电机控制量]
 *                                            ↓
 *                                    [电机混控] --> [4路PWM输出]
 *
 * 5. 混控公式 (X型布局):
 *
 *    四旋翼产生的力和力矩:
 *    - 总推力: F = F1 + F2 + F3 + F4
 *    - 俯仰力矩: M_pitch = (F1 + F3) - (F2 + F4)
 *    - 横滚力矩: M_roll = (F1 + F2) - (F3 + F4)
 *    - 偏航力矩: M_yaw = (F1 + F2) - (F3 + F4) (由旋翼反扭矩产生)
 *
 *    反解混控公式:
 *    左上电机(3) = 油门 + 俯仰 - 横滚 + 偏航
 *    左下电机(4) = 油门 - 俯仰 - 横滚 - 偏航
 *    右上电机(1) = 油门 + 俯仰 + 横滚 - 偏航
 *    右下电机(2) = 油门 - 俯仰 + 横滚 + 偏航
 *
 * 6. 飞行状态说明:
 *
 *    IDLE (空闲):
 *    - 所有电机停止
 *    - 等待解锁操作
 *
 *    NORMAL (正常飞行):
 *    - 三轴PID控制
 *    - 油门 + PID混控输出
 *
 *    FIX_HEIGHT (定高):
 *    - 三轴PID + 高度PID
 *    - 激光测距传感器保持高度
 *
 *    FAIL (故障):
 *    - 遥控器断连
 *    - 电机缓慢减速降落
 *    - 降落完成后回到IDLE
 *
 * 7. 安全保护:
 *    - 油门<50时强制停止电机 (防止误触)
 *    - 电机速度限幅 0-700 (PWM比较值)
 *    - 偏航输出限幅 ±100 (防止过度旋转)
 *    - 遥控器断连自动降落
 *
 * 8. 任务周期:
 *    - 飞行控制任务: 6ms (166Hz)
 *    - 姿态解算: 每次任务执行
 *    - PID计算: 每次任务执行
 *    - 定高PID: 24ms (每4次任务执行一次)
 *    - 电机输出: 每次任务执行
 */

#include "App_flight.h"

/* ======================== 全局变量 ======================== */

/** @brief 陀螺仪+加速度计原始数据 (16位ADC值) */
Gyro_Accel_Struct gyro_accel_data = {0};

/** @brief 欧拉角 (姿态解算结果, 单位: 度) */
Euler_struct euler_angle = {0};

/** @brief 上一次陀螺仪数据 (用于低通滤波) */
Gyro_struct last_gyro = {0};

/** @brief Z轴角速度累积 (用于偏航角积分, 单位: 度) */
float gyro_z_sum = 0;

/* 外部变量声明 */
extern Remote_Data remote_data;     /**< 遥控器数据 */
extern Flight_State flight_state;   /**< 飞行状态 */
extern TaskHandle_t com_task_handle; /**< 通信任务句柄 */

/* ======================== 电机定义 ======================== */

/**
 * @brief 左上电机
 * @note  定时器: TIM3, 通道: CH1
 *        引脚: PA6 (TIM3_CH1)
 *        旋转方向: 顺时针 (俯视)
 */
Motor_Struct left_top_motor = {.tim = &htim3, .channel = TIM_CHANNEL_1, .speed = 0};

/**
 * @brief 左下电机
 * @note  定时器: TIM4, 通道: CH4
 *        引脚: PB9 (TIM4_CH4)
 *        旋转方向: 逆时针 (俯视)
 */
Motor_Struct left_bottom_motor = {.tim = &htim4, .channel = TIM_CHANNEL_4, .speed = 0};

/**
 * @brief 右上电机
 * @note  定时器: TIM2, 通道: CH2
 *        引脚: PA1 (TIM2_CH2)
 *        旋转方向: 顺时针 (俯视)
 */
Motor_Struct right_top_motor = {.tim = &htim2, .channel = TIM_CHANNEL_2, .speed = 0};

/**
 * @brief 右下电机
 * @note  定时器: TIM1, 通道: CH3
 *        引脚: PA10 (TIM1_CH3)
 *        旋转方向: 逆时针 (俯视)
 */
Motor_Struct right_bottom_motor = {.tim = &htim1, .channel = TIM_CHANNEL_3, .speed = 0>;

/* ======================== PID控制器定义 ======================== */

/*
 * PID控制配置说明:
 *
 * 本系统采用串级PID控制:
 * - 外环: 角度环, 输入为欧拉角误差, 输出为角速度目标
 * - 内环: 角速度环, 输入为角速度误差, 输出为电机控制量
 *
 * 调参策略:
 * 1. 先调内环: 确保角速度能快速跟踪指令
 * 2. 再调外环: 确保角度能快速稳定
 * 3. 微调: 根据飞行效果微调参数
 *
 * 负号说明:
 * - 部分PID参数为负值, 用于修正控制方向
 * - 例如: pitch_pid.kp = -3.50, 表示当pitch增大时, 输出为负, 使电机减速
 */

/* ============ 俯仰轴PID (Pitch) ============ */

/**
 * @brief 俯仰轴外环PID (角度环)
 * @note  输入: 目标俯仰角 - 当前俯仰角
 *        输出: 目标Y轴角速度
 *        Kp=-3.50: 负号用于修正控制方向
 *        Ki=0: 不使用积分项 (四旋翼响应要求快)
 *        Kd=0: 不使用微分项 (外环一般不需要)
 */
PID_Struct pitch_pid = {.kp = -3.50, .ki = 0.00, .kd = 0.00};

/**
 * @brief 俯仰轴内环PID (角速度环)
 * @note  输入: 目标Y轴角速度 - 当前Y轴角速度
 *        输出: 电机控制量
 *        Kp=5.50: 响应速度
 *        Kd=0.15: 抑制角速度振荡
 */
PID_Struct gyro_y_pid = {.kp = 5.50, .ki = 0.00, .kd = 0.15};

/* ============ 横滚轴PID (Roll) ============ */

/** @brief 横滚轴外环PID (角度环) */
PID_Struct roll_pid = {.kp = -3.50, .ki = 0.00, .kd = 0.00};

/** @brief 横滚轴内环PID (角速度环) */
PID_Struct gyro_x_pid = {.kp = 5.50, .ki = 0.00, .kd = 0.15};

/* ============ 偏航轴PID (Yaw) ============ */

/**
 * @brief 偏航轴外环PID (角度环)
 * @note  偏航角通过陀螺仪积分得到, 存在累积漂移
 *        但在飞行控制中可接受
 */
PID_Struct yaw_pid = {.kp = -1.50, .ki = 0.00, .kd = 0.00};

/**
 * @brief 偏航轴内环PID (角速度环)
 * @note  Kp=-2.50: 负号用于修正控制方向
 */
PID_Struct gyro_z_pid = {.kp = -2.50, .ki = 0.00, .kd = 0.00};

/* ============ 高度PID ============ */

/**
 * @brief 高度PID
 * @note  仅在FIX_HEIGHT模式下使用
 *        输入: 目标高度 - 当前高度 (mm)
 *        输出: 高度补偿量 (叠加到油门)
 *        Kp=-1.3: 高度响应速度
 *        Kd=-0.05: 抑制高度振荡
 */
PID_Struct height_pid = {.kp = -1.3, .ki = 0.00, .kd = -0.05};

/* 定高目标高度 (进入定高模式时记录) */
extern uint16_t fix_height;

/* ======================== 飞控初始化 ======================== */

/**
 * @brief 飞控应用初始化
 * @note  初始化顺序很重要:
 *        1. MPU6050: 包含偏移校准, 需要飞机水平静止
 *        2. 电机: 初始化PWM输出, 速度设为0
 *        3. VL53L1X: 激光测距传感器
 *
 *        注意: MPU6050校准需要约2-3秒, 期间飞机必须保持水平静止
 */
void App_flight_init(void)
{
    /* 1. 初始化MPU6050 (含偏移校准) */
    Int_MPU6050_Init();

    /* 2. 启动四个电机 (PWM输出, 初始速度0) */
    Int_motor_start(&left_top_motor);
    Int_motor_start(&left_bottom_motor);
    Int_motor_start(&right_top_motor);
    Int_motor_start(&right_bottom_motor);

    /* 3. 初始化激光测距传感器 */
    Int_VL53L1X_Init();
}

/* ======================== 姿态解算 ======================== */

/**
 * @brief 获取欧拉角
 * @note  数据处理流程:
 *
 *        Step 1: 读取MPU6050原始数据
 *        - 陀螺仪: 三轴角速度 (16位ADC值)
 *        - 加速度计: 三轴加速度 (16位ADC值)
 *
 *        Step 2: 陀螺仪低通滤波
 *        - 目的: 滤除高频噪声
 *        - 方法: 一阶低通滤波 (alpha=0.15)
 *        - 特点: 响应快, 适合高频数据
 *
 *        Step 3: 加速度计卡尔曼滤波
 *        - 目的: 平滑数据, 滤除噪声
 *        - 方法: 卡尔曼滤波 (Q=0.001, R=0.543)
 *        - 特点: 平滑性好, 适合低频数据
 *
 *        Step 4: Mahony互补滤波解算欧拉角
 *        - 目的: 融合陀螺仪和加速度计数据
 *        - 方法: Mahony互补滤波 (Kp=0.8, Ki=0.0003)
 *        - 输出: 俯仰角、横滚角、偏航角 (度)
 *
 *        滤波参数说明:
 *        - ALPHA=0.15: 低通滤波权重, 越小滤波越强
 *        - Q=0.001: 卡尔曼过程噪声, 越小越信任模型
 *        - R=0.543: 卡尔曼测量噪声, 越小越信任测量
 *        - Kp=0.8: 加速度计修正权重
 *        - Ki=0.0003: 陀螺仪零偏补偿
 */
void App_flight_get_euler_angle(void)
{
    /* 1. 读取MPU6050原始数据 */
    Int_MPU6050_Get_Data(&gyro_accel_data);

    /* 2. 陀螺仪低通滤波 (alpha=0.15) */
    gyro_accel_data.gyro.gyro_x = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_x, last_gyro.gyro_x);
    gyro_accel_data.gyro.gyro_y = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_y, last_gyro.gyro_y);
    gyro_accel_data.gyro.gyro_z = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_z, last_gyro.gyro_z);
    last_gyro.gyro_x = gyro_accel_data.gyro.gyro_x;
    last_gyro.gyro_y = gyro_accel_data.gro.gyro_y;
    last_gyro.gyro_z = gyro_accel_data.gyro.gyro_z;

    /* 3. 加速度计卡尔曼滤波 (Q=0.001, R=0.543) */
    gyro_accel_data.accel.accel_x = Common_Filter_KalmanFilter(&kfs[0], gyro_accel_data.accel.accel_x);
    gyro_accel_data.accel.accel_y = Common_Filter_KalmanFilter(&kfs[1], gyro_accel_data.accel.accel_y);
    gyro_accel_data.accel.accel_z = Common_Filter_KalmanFilter(&kfs[2], gyro_accel_data.accel.accel_z);

    /* 4. Mahony互补滤波解算欧拉角 (积分时间6ms) */
    Common_IMU_GetEulerAngle(&gyro_accel_data, &euler_angle, 0.006);
}

/* ======================== PID控制 ======================== */

/**
 * @brief PID控制计算
 * @note  三轴串级PID控制:
 *
 *        数据流:
 *        遥控器指令 -> [角度映射] -> 目标角度
 *                                    ↓
 *        当前欧拉角 -> [角度误差] -> [外环PID] -> 目标角速度
 *                                                    ↓
 *        当前角速度 -> [角速度误差] -> [内环PID] -> 电机控制量
 *
 *        角度映射:
 *        - 遥控器范围: 0-1000, 500为中间值
 *        - 映射到角度: (值-500)/50 = ±10°
 *        - 例: 遥控器值600 -> 目标角度+2°
 *
 *        角速度单位转换:
 *        - 原始值: 16位ADC值 (±32768)
 *        - 转换为°/s: 值 * 2000 / 32768
 */
void App_flight_pid_process(void)
{
    /* ============ 俯仰轴PID (Pitch) ============ */

    /* 外环目标值: 遥控器指令映射到角度范围 (-10° ~ +10°) */
    pitch_pid.desire = (remote_data.pit - 500) / 50.0;

    /* 外环测量值: 当前俯仰角 (度) */
    pitch_pid.measure = euler_angle.pitch;

    /* 内环测量值: Y轴角速度 (转换为°/s) */
    gyro_y_pid.measure = (gyro_accel_data.gyro.gyro_y * 2000.0 / 32768.0);

    /* 计算串级PID */
    Com_PID_Calc_Chain(&pitch_pid, &gyro_y_pid);

    /* ============ 横滚轴PID (Roll) ============ */

    roll_pid.desire = (remote_data.rol - 500) / 50.0;
    roll_pid.measure = euler_angle.roll;
    gyro_x_pid.measure = (gyro_accel_data.gyro.gyro_x * 2000.0 / 32768.0);

    Com_PID_Calc_Chain(&roll_pid, &gyro_x_pid);

    /* ============ 偏航轴PID (Yaw) ============ */

    yaw_pid.desire = (remote_data.yaw - 500) / 50.0;
    yaw_pid.measure = euler_angle.yaw;
    gyro_z_pid.measure = (gyro_accel_data.gyro.gyro_z * 2000.0 / 32768.0);

    Com_PID_Calc_Chain(&yaw_pid, &gyro_z_pid);
}

/* ======================== 电机混控 ======================== */

/**
 * @brief 电机控制输出
 * @note  X型四旋翼混控公式:
 *
 *        混控原理:
 *        四旋翼通过改变四个电机的转速差来产生力矩, 实现姿态控制:
 *        - 俯仰: 前后电机转速差
 *        - 横滚: 左右电机转速差
 *        - 偏航: 对角电机转速差 (利用反扭矩)
 *
 *        混控公式:
 *        左上电机 = 油门 + 俯仰 - 横滚 + 偏航
 *        左下电机 = 油门 - 俯仰 - 横滚 - 偏航
 *        右上电机 = 油门 + 俯仰 + 横滚 - 偏航
 *        右下电机 = 油门 - 俯仰 + 横滚 + 偏航
 *
 *        符号说明:
 *        - 俯仰+: 抬头 (左上、右上电机加速)
 *        - 横滚+: 右倾 (右上、右下电机加速)
 *        - 偏航+: 顺时针旋转 (左上、左下电机加速)
 *
 *        飞行状态处理:
 *        - IDLE: 电机停止
 *        - NORMAL: 油门 + PID混控
 *        - FIX_HEIGHT: 油门 + PID混控 + 高度PID
 *        - FAIL: 缓慢减速降落 (每6ms减速2)
 */
void App_flight_control_motor(void)
{
    switch (flight_state)
    {
    case IDLE:
        /* 空闲状态: 电机停止 */
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
        break;

    case NORMAL:
        /* 正常飞行: 油门 + PID混控 */
        left_top_motor.speed = remote_data.thr
                             + gyro_y_pid.output - gyro_x_pid.output
                             + Com_limit(gyro_z_pid.output, 100, -100);
        left_bottom_motor.speed = remote_data.thr
                                - gyro_y_pid.output - gyro_x_pid.output
                                - Com_limit(gyro_z_pid.output, 100, -100);
        right_top_motor.speed = remote_data.thr
                              + gyro_y_pid.output + gyro_x_pid.output
                              - Com_limit(gyro_z_pid.output, 100, -100);
        right_bottom_motor.speed = remote_data.thr
                                 - gyro_y_pid.output + gyro_x_pid.output
                                 + Com_limit(gyro_z_pid.output, 100, -100);
        break;

    case FIX_HEIGHT:
        /* 定高飞行: 油门 + PID混控 + 高度PID */
        left_top_motor.speed = remote_data.thr
                             + gyro_y_pid.output - gyro_x_pid.output
                             + Com_limit(gyro_z_pid.output, 100, -100)
                             + height_pid.output;
        left_bottom_motor.speed = remote_data.thr
                                - gyro_y_pid.output - gyro_x_pid.output
                                - Com_limit(gyro_z_pid.output, 100, -100)
                                + height_pid.output;
        right_top_motor.speed = remote_data.thr
                              + gyro_y_pid.output + gyro_x_pid.output
                              - Com_limit(gyro_z_pid.output, 100, -100)
                              + height_pid.output;
        right_bottom_motor.speed = remote_data.thr
                                 - gyro_y_pid.output + gyro_x_pid.output
                                 + Com_limit(gyro_z_pid.output, 100, -100)
                                 + height_pid.output;
        break;

    case FAIL:
        /* 故障状态: 缓慢减速降落 */
        left_top_motor.speed -= 2;
        left_bottom_motor.speed -= 2;
        right_top_motor.speed -= 2;
        right_bottom_motor.speed -= 2;

        /* 检查是否所有电机已停止 */
        if (left_top_motor.speed <= 0 && left_bottom_motor.speed <= 0 &&
            right_top_motor.speed <= 0 && right_bottom_motor.speed <= 0)
        {
            /* 通知通信任务故障处理完成 */
            xTaskNotifyGive(com_task_handle);
        }
        break;

    default:
        break;
    }

    /* 限幅: 电机速度范围 0-700 */
    left_top_motor.speed = Com_limit(left_top_motor.speed, 700, 0);
    left_bottom_motor.speed = Com_limit(left_bottom_motor.speed, 700, 0);
    right_top_motor.speed = Com_limit(right_top_motor.speed, 700, 0);
    right_bottom_motor.speed = Com_limit(right_bottom_motor.speed, 700, 0);

    /* 安全保护: 油门<50时强制停止电机 */
    if (remote_data.thr < 50)
    {
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
    }

    /* 输出到电机 */
    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&right_bottom_motor);
}

/* ======================== 定高控制 ======================== */

/**
 * @brief 定高PID计算
 * @note  使用VL53L1X激光测距传感器
 *
 *        工作原理:
 *        - 激光传感器测量地面距离 (mm)
 *        - 记录进入定高模式时的高度作为目标
 *        - 通过PID控制保持当前高度
 *
 *        执行周期:
 *        - 24ms执行一次 (激光传感器采样周期)
 *        - 在6ms的飞行控制任务中, 每4次执行一次
 *
 *        PID配置:
 *        - Kp=-1.3: 高度响应速度
 *        - Ki=0: 不使用积分项
 *        - Kd=-0.05: 抑制高度振荡
 *
 *        输出:
 *        - 高度补偿量, 叠加到油门值
 *        - 正值: 电机加速 (高度低于目标)
 *        - 负值: 电机减速 (高度高于目标)
 */
void App_flight_fix_height_pid_process(void)
{
    /* 设置目标高度 (进入定高模式时记录的高度) */
    height_pid.desire = fix_height;

    /* 设置当前高度 (激光测距传感器测量值, 单位mm) */
    height_pid.measure = Int_VL53L1X_GetDistance();

    /* 计算高度PID */
    Com_PID_Calc(&height_pid);
}
