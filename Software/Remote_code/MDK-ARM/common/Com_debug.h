/**
 * @file    Com_debug.h
 * @brief   调试打印模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了调试打印模块的公共接口：
 *          - DEBUG_LOG_ENABLE宏：控制调试输出开关
 *          - debug_printf()宏：带文件名和行号的调试打印
 *
 * 功能特性：
 * - 自动添加文件名和行号前缀
 * - 支持可变参数
 * - 可通过宏定义全局开关
 * - 重定向printf()到USART1
 *
 * 输出格式：
 * [filename:line]  message
 *
 * 示例输出：
 * [main.c:42]  Hello remote!
 * [App_process_data.c:100]  value: 123
 *
 * 使用方法：
 * @code
 * // 基本使用
 * debug_printf("Hello World!\n");
 *
 * // 带变量
 * int value = 123;
 * debug_printf("value: %d\n", value);
 *
 * // 带多个变量
 * float temp = 25.5;
 * debug_printf("temp: %.1f, value: %d\n", temp, value);
 * @endcode
 *
 * @note   调试输出会占用CPU时间（每10字节约1ms）
 * @note   发布版本应设置DEBUG_LOG_ENABLE=0禁用输出
 * @note   printf()通过fputc()重定向到USART1
 */

#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

/* ======================== 头文件引用 ======================== */

#include "usart.h"      /* USART驱动 */
#include "stdio.h"      /* 标准输入输出 */
#include "stdarg.h"     /* 可变参数支持 */
#include <string.h>     /* 字符串函数 */

/* ======================== 调试开关 ======================== */

/**
 * @brief 调试输出使能宏
 *
 * 设置为1：启用调试输出
 * 设置为0：禁用调试输出（编译为空操作）
 *
 * @note   发布版本应设置为0，减少CPU占用
 * @note   调试版本建议设置为1
 */
#define DEBUG_LOG_ENABLE    1

/* ======================== 调试打印宏定义 ======================== */

#ifdef DEBUG_LOG_ENABLE

/**
 * @brief 提取文件名宏（去除路径）
 *
 * 从完整路径中提取文件名：
 * - "C:\project\src\main.c" -> "main.c"
 * - "/home/user/project/main.c" -> "main.c"
 *
 * 支持Windows(\)和Linux(/)路径分隔符
 */
#define __FILE_NAME__  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILE_NAME    (strrchr(__FILE_NAME__, '/') ? strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)

/**
 * @brief 调试打印宏
 *
 * 输出格式：[filename:line]  message
 *
 * 实现原理：
 * - 使用printf()输出
 * - 自动添加文件名和行号前缀
 * - 支持可变参数
 *
 * @param  format 格式字符串（与printf()相同）
 * @param  ... 可变参数
 *
 * @code
 * // 使用示例
 * debug_printf("Hello World!\n");
 * debug_printf("value: %d, name: %s\n", 123, "test");
 * @endcode
 *
 * @note   输出会占用CPU时间，频繁调用影响实时性
 * @note   建议仅在调试阶段使用
 */
#define debug_printf(format, ...) printf("[%s:%d]  " format, __FILE_NAME, __LINE__, ##__VA_ARGS__)

#else

/* 调试禁用 - 编译为空操作 */
#define debug_printf(format, ...)

#endif /* DEBUG_LOG_ENABLE */

#endif /* __COM_DEBUG_H__ */
