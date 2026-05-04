/**
 * @file    Int_IP5305T.h
 * @brief   IP5305T电源管理模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了IP5305T电源管理模块的公共接口：
 *          - Int_IP5305T_start()函数声明
 *
 * IP5305T芯片特性：
 * - 集成充放电管理
 * - 内置升压转换器
 * - 支持边充边放
 * - 自动关机功能（无负载30秒后关机）
 * - 按键开关机
 *
 * 本模块通过定期触发电源按键来防止自动关机：
 * - 每10秒发送一个100ms的低脉冲
 * - 模拟用户按下电源键
 * - 保持电源开启状态
 *
 * 硬件连接：
 * - POWER_KEY -> PB15（低电平有效）
 * - 按键接地，内部上拉
 *
 * 使用示例：
 * @code
 * // 在电源保持任务中调用
 * void power_task(void *args)
 * {
 *     TickType_t xLastWakeTime = xTaskGetTickCount();
 *     while(1)
 *     {
 *         vTaskDelayUntil(&xLastWakeTime, 10000);  // 10秒周期
 *         Int_IP5305T_start();  // 发送保持脉冲
 *     }
 * }
 * @endcode
 *
 * @note   必须每10秒触发一次，否则芯片自动关机
 * @note   按键脉冲宽度100ms，符合IP5305T时序要求
 */

#ifndef __INT_IP5305T__
#define __INT_IP5305T__

/* ======================== 头文件引用 ======================== */

#include "main.h"           /* 主头文件（GPIO定义） */
#include "FreeRTOS.h"       /* FreeRTOS内核 */
#include "task.h"           /* FreeRTOS任务 */

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  发送电源保持脉冲
 *
 * @details 生成100ms的低脉冲触发IP5305T电源按键：
 *          1. 拉低POWER_KEY引脚（模拟按键按下）
 *          2. 保持100ms（IP5305T要求>50ms）
 *          3. 拉高POWER_KEY引脚（模拟按键释放）
 *
 * @note   此函数在FreeRTOS任务中调用，使用vTaskDelay()
 * @note   脉冲宽度100ms，满足IP5305T按键检测要求
 * @note   调用周期应<30秒，防止自动关机
 */
void Int_IP5305T_start(void);

#endif /* __INT_IP5305T__ */
