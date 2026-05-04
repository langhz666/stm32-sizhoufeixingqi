/**
 * @file    App_process_data.h
 * @brief   数据处理模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了数据处理模块的公共接口：
 *          - Remote_Data数据结构
 *          - App_process_key_data()函数声明
 *          - App_process_joystick_data()函数声明
 *
 * 数据结构说明：
 * Remote_Data包含处理后的摇杆值（0-1000范围）和控制标志
 * 通过extern在其他模块中访问
 *
 * 使用示例：
 * @code
 * // 在其他模块中访问遥控器数据
 * extern Remote_Data remote_data;
 * int16_t throttle = remote_data.thr;
 * @endcode
 *
 * @note   摇杆值范围：0-1000，500为中位
 * @note   控制标志在发送后自动清零
 */

#ifndef __APP_PROCESS_DATA__
#define __APP_PROCESS_DATA__

/* ======================== 头文件引用 ======================== */

/* 驱动层头文件 */
#include "Int_joystick.h"       /* 摇杆ADC驱动 */
#include "Int_key.h"            /* 按键驱动 */

/* 通用工具头文件 */
#include "Com_debug.h"          /* 调试打印 */
#include "Com_tool.h"           /* 工具函数（限幅等） */

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 遥控器数据结构
 *
 * 包含处理后的摇杆值和控制标志：
 * - thr/yaw/pit/rol: 摇杆值，范围0-1000
 * - shutdown: 关机标志
 * - fix_height: 定高标志
 *
 * 摇杆值说明：
 * - THR（油门）：0=无油门，1000=最大油门
 * - YAW（偏航）：0=左转，500=中位，1000=右转
 * - PIT（俯仰）：0=后退，500=中位，1000=前进
 * - ROL（横滚）：0=左倾，500=中位，1000=右倾
 *
 * 控制标志说明：
 * - shutdown: 1=发送关机指令，0=无操作（发送后自动清零）
 * - fix_height: 1=切换定高模式，0=无操作（发送后自动清零）
 */
typedef struct
{
    int16_t thr;        /**< 油门值：0-1000 */
    int16_t yaw;        /**< 偏航值：0-1000，500为中位 */
    int16_t pit;        /**< 俯仰值：0-1000，500为中位 */
    int16_t rol;        /**< 横滚值：0-1000，500为中位 */
    uint8_t shutdown;   /**< 关机标志：1=关机，0=无操作 */
    uint8_t fix_height; /**< 定高标志：1=切换定高，0=无操作 */
} Remote_Data;

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  处理按键输入事件
 *
 * @details 扫描按键并处理相应事件：
 *          - 方向键：调节Pitch/Roll微调值（每次±10）
 *          - LEFT_X键：设置关机标志
 *          - RIGHT_X键：短按切换定高，长按校准摇杆
 *
 * @note   此函数在key_task中每20ms调用一次
 * @note   微调值范围建议在±200以内
 */
void App_process_key_data(void);

/**
 * @brief  处理摇杆ADC数据
 *
 * @details 完整的数据处理流水线：
 *          1. 读取原始ADC值（通过DMA）
 *          2. 范围映射：0-4095 → 0-1000
 *          3. 应用校准偏移
 *          4. 应用按键微调
 *          5. 限幅处理
 *          6. 复制到remote_data结构
 *
 * @note   此函数在joy_task中每20ms调用一次
 * @note   数据在临界区内处理，防止多任务冲突
 */
void App_process_joystick_data(void);

#endif /* __APP_PROCESS_DATA__ */
