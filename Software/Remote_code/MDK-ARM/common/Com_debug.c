/**
 * @file    Com_debug.c
 * @brief   调试打印模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块实现printf()函数的重定向：
 *          - 将printf()输出重定向到USART1
 *          - 支持debug_printf()宏，自动添加文件名和行号
 *          - 用于开发调试阶段的日志输出
 *
 * 使用方法：
 * - debug_printf("value: %d\n", var);
 * - 输出格式：[main.c:42]  value: 123
 *
 * 硬件配置：
 * - USART1: PA9(TX), PA10(RX)
 * - 波特率: 115200
 * - 数据位: 8
 * - 停止位: 1
 * - 校验: 无
 *
 * @note   调试输出会占用CPU时间（每10字节约1ms）
 * @note   发布版本应设置DEBUG_LOG_ENABLE=0禁用输出
 * @note   printf()通过fputc()重定向到UART
 */

#include "Com_debug.h"

/**
 * @brief  重定向fputc函数
 *
 * @param  ch 要发送的字符
 * @param  f 文件指针（未使用）
 * @return 发送的字符
 *
 * @details printf()调用链：
 *          printf() -> putchar() -> fputc() -> HAL_UART_Transmit()
 *
 *          重定向实现：
 *          1. 接收printf()传递的字符
 *          2. 通过USART1发送出去
 *          3. 返回字符（printf()要求）
 *
 * @note   超时时间1000ms，防止死锁
 * @note   每次发送1个字节，效率较低但简单可靠
 */
int fputc(int ch, FILE *f)
{
    /* 通过USART1发送单个字符 */
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);

    /* 返回字符（printf()要求） */
    return ch;
}
