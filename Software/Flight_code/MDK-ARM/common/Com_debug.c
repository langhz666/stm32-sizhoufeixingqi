/**
 * @file    Com_debug.c
 * @brief   调试打印模块实现 - 重定向printf到串口
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过重定向fputc函数实现printf输出到USART2
 */

#include "Com_debug.h"

/**
 * @brief 重定向fputc函数到USART2
 * @param ch    要发送的字符
 * @param f     文件指针 (未使用)
 * @return      发送的字符
 * @note        此函数被printf内部调用，实现串口输出
 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);
    return ch;
}
