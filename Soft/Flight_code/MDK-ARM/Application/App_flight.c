#include "App_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Euler_struct euler_angle = {0};
Gyro_struct last_gyro = {0};
float gyro_z_sum = 0;

extern Remote_Data remote_data;
extern Flight_State flight_state;
extern TaskHandle_t com_task_handle;
// 电机结构体
Motor_Struct left_top_motor = {.tim = &htim3, .channel = TIM_CHANNEL_1, .speed = 0};
Motor_Struct left_bottom_motor = {.tim = &htim4, .channel = TIM_CHANNEL_4, .speed = 0};
Motor_Struct right_top_motor = {.tim = &htim2, .channel = TIM_CHANNEL_2, .speed = 0};
Motor_Struct right_bottom_motor = {.tim = &htim1, .channel = TIM_CHANNEL_3, .speed = 0};

// PID的调参是先调节内环再调节外环
// 俯仰角PID结构体  => 后续需要进行专业的PID调参
PID_Struct pitch_pid = {.kp = -3.50, .ki = 0.00, .kd = 0.00};
// Y轴角速度结构体 => 对应俯仰角的内环
// 极性问题 => 参数的正负可以调节 => 作用于电机的时候 正负
PID_Struct gyro_y_pid = {.kp = 5.50, .ki = 0.00, .kd = 0.15};

// 横滚角PID结构体
PID_Struct roll_pid = {.kp = -3.50, .ki = 0.00, .kd = 0.00};
// X轴角速度结构体 => 对应横滚角的内环
PID_Struct gyro_x_pid = {.kp = 5.50, .ki = 0.00, .kd = 0.15};

// 偏航角属于辅助功能 => 并不在平稳飞行的范围  只需要保证飞机不要在空中旋转即可 => 明显的返回就行
// 偏航角PID结构体  => 可以只通过比例部分来实现
PID_Struct yaw_pid = {.kp = -1.50, .ki = 0.00, .kd = 0.00};
// Z轴角速度结构体 => 对应偏航角的内环
PID_Struct gyro_z_pid = {.kp = -2.50, .ki = 0.00, .kd = 0.00};

// 定高的PID结构体
PID_Struct height_pid = {.kp = -1.3, .ki = 0.00, .kd = -0.05};

// 记录下的定高飞行高度
extern uint16_t fix_height;

/**
 * @brief 飞控任务初始化 MPU6050初始化    启动电机
 *
 */
void App_flight_init(void)
{
    Int_MPU6050_Init();

    // 启动电机
    Int_motor_start(&left_top_motor);
    Int_motor_start(&left_bottom_motor);
    Int_motor_start(&right_top_motor);
    Int_motor_start(&right_bottom_motor);

    // 初始化激光测距仪
    Int_VL53L1X_Init();
}

/**
 * @brief 根据陀螺仪测量的数据 计算出欧拉角
 *
 */
void App_flight_get_euler_angle(void)
{
    // 1. 使用MPU6050的硬件接口  得到六轴数据
    Int_MPU6050_Get_Data(&gyro_accel_data);

    // 2. 对角速度进行低通滤波  =>  后续对采集数据的使用是及时性比较高的
    // 对准确性要求没那么高  但是一定要计算迅速
    // output = 加权系数 * last_output + ( 1 - 加权系数 )* 本次的测量值;
    gyro_accel_data.gyro.gyro_x = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_x, last_gyro.gyro_x);
    gyro_accel_data.gyro.gyro_y = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_y, last_gyro.gyro_y);
    gyro_accel_data.gyro.gyro_z = Common_Filter_LowPass(gyro_accel_data.gyro.gyro_z, last_gyro.gyro_z);
    last_gyro.gyro_x = gyro_accel_data.gyro.gyro_x;
    last_gyro.gyro_y = gyro_accel_data.gyro.gyro_y;
    last_gyro.gyro_z = gyro_accel_data.gyro.gyro_z;

    // 先打印角速度
    // debug_printf(":%d,%d,%d\n", gyro_accel_data.gyro.gyro_x, gyro_accel_data.gyro.gyro_y, gyro_accel_data.gyro.gyro_z);

    // 3. 对波动变化比较大的加速度 使用更高级的滤波方式 => 卡尔曼滤波
    gyro_accel_data.accel.accel_x = Common_Filter_KalmanFilter(&kfs[0], gyro_accel_data.accel.accel_x);
    gyro_accel_data.accel.accel_y = Common_Filter_KalmanFilter(&kfs[1], gyro_accel_data.accel.accel_y);
    gyro_accel_data.accel.accel_z = Common_Filter_KalmanFilter(&kfs[2], gyro_accel_data.accel.accel_z);

    // 打印加速度
    // debug_printf(":%d,%d,%d\n", gyro_accel_data.accel.accel_x, gyro_accel_data.accel.accel_y, gyro_accel_data.accel.accel_z);

    // 4. 通过加速度和角速度来计算当前飞机切斜的角度 => 姿态解算
    // 使用互补解算计算欧拉角 => 优先使用加速度解算 => 俯仰角和横滚角能够使用
    // euler_angle.pitch = atan2(gyro_accel_data.accel.accel_x * 1.0, gyro_accel_data.accel.accel_z) / 3.14159 * 180;

    // euler_angle.roll = atan2(gyro_accel_data.accel.accel_y * 1.0, gyro_accel_data.accel.accel_z) / 3.14159 * 180;

    // // 偏航角 => 只能使用角速度积分
    // // 16位ADC的值转换为°/s  => 量程是±2000°/s
    // gyro_z_sum += (gyro_accel_data.gyro.gyro_z * 2000.0 / 32768.0) * 0.006;
    // euler_angle.yaw = gyro_z_sum;

    // 也可以使用移植的四元数姿态解算
    Common_IMU_GetEulerAngle(&gyro_accel_data, &euler_angle, 0.006);

    // 俯仰角  横滚角  偏航角
    // debug_printf(":%.2f,%.2f,%.2f\n", euler_angle.pitch, euler_angle.roll, euler_angle.yaw);
}

/**
 * @brief 根据欧拉角 计算出PID的目标值
 *
 */
void App_flight_pid_process(void)
{
    // 俯仰角
    // 1. 需要赋值目标值和测量值
    // 外环的目标角度 => 如果是平稳飞行 => 值为0 => 如果需要遥控飞行 => 目标角度就是遥控器的值
    // 数值转换 => remote_data.pit(0-1000,500为中间点)  控制范围在±10°
    pitch_pid.desire = (remote_data.pit - 500) / 50.0;
    // 内环的测量值 => 就是当前的俯仰角
    pitch_pid.measure = euler_angle.pitch;
    // 外环的测量值 => 当前的角速度  => 单位要保持一致
    gyro_y_pid.measure = (gyro_accel_data.gyro.gyro_y * 2000.0 / 32768.0);

    // 2. 进行PID计算
    Com_PID_Calc_Chain(&pitch_pid, &gyro_y_pid);

    // 先观察内环  => 角速度控制 => 目标位角速度为0
    // debug_printf(":%.2f,%.2f\n", gyro_y_pid.err, gyro_y_pid.output);

    // 横滚角
    // 1. 需要赋值目标值和测量值
    // 外环的目标角度 => 如果是平稳飞行 => 值为0 => 如果需要遥控飞行 => 目标角度就是遥控器的值
    roll_pid.desire = (remote_data.rol - 500) / 50.0;
    // 内环的测量值
    roll_pid.measure = euler_angle.roll;
    // 外环的测量值
    gyro_x_pid.measure = (gyro_accel_data.gyro.gyro_x * 2000.0 / 32768.0);

    // 2. 进行PID计算
    Com_PID_Calc_Chain(&roll_pid, &gyro_x_pid);

    // 偏航角
    // 1. 需要赋值目标值和测量值
    yaw_pid.desire = (remote_data.yaw - 500) / 50.0;
    // 内环的测量值
    yaw_pid.measure = euler_angle.yaw;
    // 外环的测量值
    gyro_z_pid.measure = (gyro_accel_data.gyro.gyro_z * 2000.0 / 32768.0);

    // 2. 进行PID计算
    Com_PID_Calc_Chain(&yaw_pid, &gyro_z_pid);
}

/**
 * @brief 根据PID的输出值 控制电机
 *
 */
void App_flight_control_motor(void)
{
    // 1. 首先判断当前飞机的飞行状态
    switch (flight_state)
    {
    case IDLE:
        // 一旦进入加锁状态 =>  需要将电机速度设置为0
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
        break;
    case NORMAL:
        // 俯仰角 => 向前飞有角速度 => 正误差 => 需要一个向后飞的反馈效果 => 前两个电机转的快 后两个转的慢
        // 不同重要程度的PID控制结果 可以进行适当的限制
        left_top_motor.speed = remote_data.thr + gyro_y_pid.output - gyro_x_pid.output + Com_limit(gyro_z_pid.output, 100, -100);
        left_bottom_motor.speed = remote_data.thr - gyro_y_pid.output - gyro_x_pid.output - Com_limit(gyro_z_pid.output, 100, -100);
        right_top_motor.speed = remote_data.thr + gyro_y_pid.output + gyro_x_pid.output - Com_limit(gyro_z_pid.output, 100, -100);
        right_bottom_motor.speed = remote_data.thr - gyro_y_pid.output + gyro_x_pid.output + Com_limit(gyro_z_pid.output, 100, -100);
        break;
    case FIX_HEIGHT:
        // 只有定高状态才需要进行定高的PID计算  => 定高同时也需要平稳飞行
        left_top_motor.speed = remote_data.thr + gyro_y_pid.output - gyro_x_pid.output + Com_limit(gyro_z_pid.output, 100, -100) + height_pid.output;
        left_bottom_motor.speed = remote_data.thr - gyro_y_pid.output - gyro_x_pid.output - Com_limit(gyro_z_pid.output, 100, -100) + height_pid.output;
        right_top_motor.speed = remote_data.thr + gyro_y_pid.output + gyro_x_pid.output - Com_limit(gyro_z_pid.output, 100, -100) + height_pid.output;
        right_bottom_motor.speed = remote_data.thr - gyro_y_pid.output + gyro_x_pid.output + Com_limit(gyro_z_pid.output, 100, -100) + height_pid.output;
        break;
    case FAIL:
        // 进行故障处理 => 一直处理  到满足条件 修改状态为IDLE
        // 6ms => 降低速度2点
        left_top_motor.speed -= 2;
        left_bottom_motor.speed -= 2;
        right_top_motor.speed -= 2;
        right_bottom_motor.speed -= 2;
        if (left_top_motor.speed <= 0 && left_bottom_motor.speed <= 0 && right_top_motor.speed <= 0 && right_bottom_motor.speed <= 0)
        {
            // 故障处理完成  电机转速都已经降低为0
            xTaskNotifyGive(com_task_handle);
        }

        break;
    default:
        break;
    }

    // 限制电机速度的上限值
    // 可以通过提供速度上限  让飞行更加平稳
    left_top_motor.speed = Com_limit(left_top_motor.speed, 700, 0);
    left_bottom_motor.speed = Com_limit(left_bottom_motor.speed, 700, 0);
    right_top_motor.speed = Com_limit(right_top_motor.speed, 700, 0);
    right_bottom_motor.speed = Com_limit(right_bottom_motor.speed, 700, 0);

    // 安全限制 => 当油门设置为<50时 => 强制将速度设置为0
    if (remote_data.thr < 50)
    {
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
    }

    // 2. 设置电机速度
    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&right_bottom_motor);
}

/**
 * @brief 进入定高功能之后的PID计算
 *
 */
void App_flight_fix_height_pid_process(void)
{
    // 24ms一次
    // 1. 填写目标值(按下定高功能时的高度)和测量值(当前激光测距仪得到的高度)
    height_pid.desire = fix_height;
    height_pid.measure = Int_VL53L1X_GetDistance();

    // 2. 进行单环PID计算  得到输出值
    Com_PID_Calc(&height_pid);
}
