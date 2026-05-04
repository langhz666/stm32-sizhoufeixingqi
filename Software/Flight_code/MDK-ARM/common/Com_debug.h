/**
 * @file    Com_debug.h
 * @brief   调试打印模块 - 提供带文件名和行号的日志打印功能
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过DEBUG_LOG_ENABLE宏控制日志输出，飞行时建议关闭以节省CPU资源
 */

#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h>

/**
 * @brief 日志打印使能开关
 * @note  设为1开启日志打印，设为0关闭
 *        日志打印会占用较多CPU资源，飞行时建议关闭
 */
#define DEBUG_LOG_ENABLE 1

#ifdef DEBUG_LOG_ENABLE

/* 从完整路径中提取文件名 */
#define __FILE_NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILE_NAME (strrchr(__FILE_NAME__, '/') ? strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)

/**
 * @brief 带文件名和行号的调试打印宏
 * @note  输出格式: [文件名:行号] 内容
 *        示例: debug_printf("value=%d\r\n", val);
 */
#define debug_printf(format, ...) printf("[%s:%d]  " format, __FILE_NAME, __LINE__, ##__VA_ARGS__)

#else

/* 日志打印关闭时的空实现 */
#define debug_printf(format, ...)

#endif /* DEBUG_LOG_ENABLE */

#endif /* __COM_DEBUG_H__ */
