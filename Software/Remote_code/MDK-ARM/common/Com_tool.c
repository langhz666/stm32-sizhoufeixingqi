/**
 * @file    Com_tool.c
 * @brief   通用工具函数模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块提供常用的工具函数：
 *          - 数值限幅函数
 *          - 其他通用工具函数（待扩展）
 *
 * 使用场景：
 * - 摇杆数据处理时限制范围（0-1000）
 * - PID输出限幅
 * - 其他需要范围限制的场合
 */

#include "Com_tool.h"

/**
 * @brief  数值限幅函数
 *
 * @param  value 输入值
 * @param  min 最小值
 * @param  max 最大值
 * @return 限幅后的值
 *
 * @details 将输入值限制在[min, max]范围内：
 *          - 如果value > max，返回max
 *          - 如果value < min，返回min
 *          - 否则返回value本身
 *
 * 算法实现：
 * 1. 保存原始值
 * 2. 检查是否超过上限
 * 3. 检查是否低于下限
 * 4. 返回限幅后的值
 *
 * @note   函数不会修改原始值，返回新值
 * @note   适用于int16_t类型数据
 * @note   典型应用：摇杆数据(0-1000)、PID输出等
 *
 * @code
 * // 示例：限制摇杆数据范围
 * int16_t thr = 1200;
 * thr = Com_limit(thr, 0, 1000);  // thr = 1000
 * @endcode
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max)
{
    int16_t result = value;

    /* 检查是否超过上限 */
    if (value > max)
    {
        result = max;
    }
    /* 检查是否低于下限 */
    else if (value < min)
    {
        result = min;
    }

    return result;
}
