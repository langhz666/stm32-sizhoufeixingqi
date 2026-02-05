#include "Com_pid.h"

// 单次PID计算
void Com_PID_Calc(PID_Struct *pid)
{
    // 1. 目标和测量 => 计算误差值
    pid->err = pid->measure - pid->desire;

    // 2. 计算积分误差
    pid->integral += pid->err;

    if (pid->last_err == 0)
    {
        pid->last_err = pid->err;
    }

    // 3. 计算微分误差
    float der = pid->err - pid->last_err;

    // 4. 计算输出
    pid->output = pid->kp * pid->err + (pid->ki * pid->integral * PID_PERIOD) + (pid->kd * der / PID_PERIOD);

    // 5. 保存上一次误差
    pid->last_err = pid->err;
}

// 串级PID计算
void Com_PID_Calc_Chain(PID_Struct *out_pid, PID_Struct *in_pid)
{
    // 1.先计算外环
    Com_PID_Calc(out_pid);
    // 2.将外环的输出值作为内环的目标值
    in_pid->desire = out_pid->output;
    // 3. 计算内环
    Com_PID_Calc(in_pid);
}

/**
 * @brief 限制数值在正常的范围内
 *
 * @param speed
 * @param max_speed
 * @param min_speed
 * @return int16_t
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
