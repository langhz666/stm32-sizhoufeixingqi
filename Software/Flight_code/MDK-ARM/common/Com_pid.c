/**
 * @file    Com_pid.c
 * @brief   PID控制算法实现
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details PID控制原理详解:
 *
 * 1. PID控制器基本公式:
 *    output = Kp * e(t) + Ki * ∫e(t)dt + Kd * de(t)/dt
 *
 *    其中:
 *    - e(t) = measure - desire (误差)
 *    - Kp: 比例系数, 决定响应速度
 *    - Ki: 积分系数, 消除静态误差
 *    - Kd: 微分系数, 抑制超调, 提供阻尼
 *
 * 2. 各参数作用详解:
 *
 *    比例项 (P): output = Kp * err
 *    - 作用: 根据当前误差大小进行修正
 *    - 特点: 响应快, 但存在静态误差
 *    - 调参: 增大Kp -> 响应更快, 但可能振荡
 *
 *    积分项 (I): output = Ki * ∫err dt
 *    - 作用: 消除静态误差 (稳态误差)
 *    - 特点: 响应慢, 可能导致超调
 *    - 调本项目中Ki=0, 不使用积分项 (四旋翼响应要求快)
 *
 *    微分项 (D): output = Kd * derr/dt
 *    - 作用: 预测误差变化趋势, 提供阻尼
 *    - 特点: 抑制超调, 但会放大噪声
 *    - 调参: 增大Kd -> 超调减小, 但噪声增大
 *
 * 3. 串级PID控制 (本项目采用):
 *
 *    结构: 外环(角度) + 内环(角速度)
 *
 *    外环 (角度环):
 *    - 输入: 目标角度 - 当前角度 = 角度误差
 *    - 输出: 目标角速度
 *    - 作用: 将角度误差转换为角速度指令
 *
 *    内环 (角速度环):
 *    - 输入: 目标角速度 - 当前角速度 = 角速度误差
 *    - 输出: 电机控制量
 *    - 作用: 快速跟踪角速度指令
 *
 *    优势:
 *    - 内环响应快, 能快速抑制角速度扰动
 *    - 外环负责稳态精度
 *    - 比单级PID具有更好的抗干扰性能
 *
 * 4. 本项目PID配置:
 *
 *    俯仰轴 (Pitch):
 *    - 外环: Kp=-3.50, Ki=0, Kd=0 (纯比例控制)
 *    - 内环: Kp=5.50, Ki=0, Kd=0.15 (比例+微分)
 *
 *    横滚轴 (Roll):
 *    - 外环: Kp=-3.50, Ki=0, Kd=0
 *    - 内环: Kp=5.50, Ki=0, Kd=0.15
 *
 *    偏航轴 (Yaw):
 *    - 外环: Kp=-1.50, Ki=0, Kd=0
 *    - 内环: Kp=-2.50, Ki=0, Kd=0
 *
 *    高度环 (Height):
 *    - Kp=-1.3, Ki=0, Kd=-0.05
 *
 * 5. 调参建议:
 *
 *    步骤1: 先调内环
 *    - 从小Kp开始, 逐渐增大
 *    - 直到角速度能快速跟踪指令
 *    - 加入适当的Kd抑制振荡
 *
 *    步骤2: 再调外环
 *    - 从小Kp开始, 逐渐增大
 *    - 直到角度能快速稳定
 *    - 外环一般不需要Kd
 *
 *    步骤3: 微调
 *    - 观察飞行效果, 微调参数
 *    - 如果振荡: 减小Kp或增大Kd
 *    - 如果响应慢: 增大Kp
 *    - 如果有静态误差: 适当增加Ki
 *
 * 6. 输出限幅:
 *    - 电机速度范围: 0-700 (PWM比较值)
 *    - 偏航限制: ±100 (防止过度旋转)
 *    - 油门<50时强制停止 (安全保护)
 */

#include "Com_pid.h"

/**
 * @brief 单级PID计算
 * @param pid   PID结构体指针
 *
 * @note    计算公式:
 *          output = Kp * err + Ki * ∫err*dt + Kd * derr/dt
 *
 *          其中:
 *          - err = measure - desire
 *          - ∫err = integral (累积)
 *          - derr/dt = (err - last_err) / dt
 *
 *          本项目特殊处理:
 *          - 积分项乘以 PID_PERIOD (6ms) 进行时间积分
 *          - 微分项除以 PID_PERIOD 进行时间微分
 *          - 首次运行时初始化 last_err = err (避免微分冲击)
 */
void Com_PID_Calc(PID_Struct *pid)
{
    /* 1. 计算误差: err = 测量值 - 目标值 */
    pid->err = pid->measure - pid->desire;

    /* 2. 累积积分项: integral += err */
    pid->integral += pid->err;

    /* 首次运行时初始化 last_err (避免微分项产生冲击) */
    if (pid->last_err == 0)
    {
        pid->last_err = pid->err;
    }

    /* 3. 计算微分项: der = err - last_err (误差变化率) */
    float der = pid->err - pid->last_err;

    /* 4. 计算PID输出:
     *    output = Kp*err + Ki*integral*dt + Kd*der/dt
     *
     *    注意单位:
     *    - Ki项乘以PID_PERIOD进行时间积分
     *    - Kd项除以PID_PERIOD进行时间微分
     */
    pid->output = pid->kp * pid->err
                + (pid->ki * pid->integral * PID_PERIOD)
                + (pid->kd * der / PID_PERIOD);

    /* 5. 保存当前误差供下次使用 */
    pid->last_err = pid->err;
}

/**
 * @brief 串级PID计算 (外环 + 内环)
 * @param out_pid   外环PID结构体指针 (角度环)
 * @param in_pid    内环PID结构体指针 (角速度环)
 *
 * @note    串级PID计算流程:
 *
 *          Step 1: 计算外环PID (角度环)
 *          - 输入: 目标角度 - 当前角度 = 角度误差
 *          - 输出: 目标角速度
 *
 *          Step 2: 将外环输出作为内环目标值
 *          - in_pid.desire = out_pid.output
 *
 *          Step 3: 计算内环PID (角速度环)
 *          - 输入: 目标角速度 - 当前角速度 = 角速度误差
 *          - 输出: 电机控制量
 *
 *          数据流:
 *          目标角度 -> [外环PID] -> 目标角速度 -> [内环PID] -> 电机控制量
 *                        ↑                           ↑
 *                     当前角度                     当前角速度
 */
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid)
{
    /* 1. 计算外环PID (角度环) */
    Com_PID_Calc(out_pid);

    /* 2. 外环输出作为内环目标值 */
    in_pid->desire = out_pid->output;

    /* 3. 计算内环PID (角速度环) */
    Com_PID_Calc(in_pid);
}

/**
 * @brief 限幅函数
 * @param speed      输入值
 * @param max_speed  最大值
 * @param min_speed  最小值
 * @return  限幅后的值
 *
 * @note    用途:
 *          - 电机速度限幅: 0-700 (PWM比较值)
 *          - 偏航输出限幅: ±100 (防止过度旋转)
 *          - 防止执行器饱和导致控制失效
 */
int16_t Com_limit(int16_t speed, int16_t max_speed, int16_t min_speed)
{
    if (speed > max_speed)
    {
        return max_speed;
    }
    else if (speed < min_speed)
    {
        return min_speed;
    }
    return speed;
}
