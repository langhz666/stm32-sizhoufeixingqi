/**
 * @file    Com_tool.h
 * @brief   通用工具函数模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了通用工具函数的公共接口：
 *          - Com_limit()函数声明
 *
 * 工具函数列表：
 * - Com_limit(): 数值限幅函数
 *
 * 使用场景：
 * - 摇杆数据处理时限制范围（0-1000）
 * - PID输出限幅
 * - 其他需要范围限制的场合
 *
 * 使用示例：
 * @code
 * // 限制摇杆数据范围
 * int16_t thr = 1200;
 * thr = Com_limit(thr, 0, 1000);  // thr = 1000
 *
 * // 限制PID输出
 * int16_t pid_output = -500;
 * pid_output = Com_limit(pid_output, -1000, 1000);  // pid_output = -500
 * @endcode
 */

#ifndef __COM_TOOL__
#define __COM_TOOL__

/* ======================== 头文件引用 ======================== */

#include "main.h"   /* 主头文件（基本类型定义） */

/* ======================== 公共函数声明 ======================== */

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
 *
 * // 示例：限制PID输出
 * int16_t output = -500;
 * output = Com_limit(output, -1000, 1000);  // output = -500
 * @endcode
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max);

#endif /* __COM_TOOL__ */
