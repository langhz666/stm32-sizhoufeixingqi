/**
 * @file    Int_IP5305T.h
 * @brief   IP5305T电源管理模块 - 自动开关机控制
 * @author  langhz666
 * @date    2025-09-27
 * @note    IP5305T是一款集成升压转换器和锂电池充电管理的芯片
 *          通过控制POWER_KEY引脚实现自动开关机
 */

#ifndef __INT_IP5305T__
#define __INT_IP5305T__

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 启动IP5305T电源 (自动开机)
 * @note  短按电源键一次，保持100ms
 */
void Int_IP5305T_start(void);

/**
 * @brief 执行关机指令
 * @note  短按两次电源键实现关机
 *        第一次短按100ms，等待200ms，第二次短按100ms
 */
void Int_IP5305T_shutdown(void);

#endif /* __INT_IP5305T__ */
