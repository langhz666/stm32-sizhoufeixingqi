/**
 * @file    Int_key.h
 * @brief   按键扫描模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了按键扫描模块的公共接口：
 *          - Key_type按键类型枚举
 *          - Int_key_get()函数声明
 *
 * 按键布局：
 * - 方向键：UP、DOWN、LEFT、RIGHT（GPIO低电平有效）
 * - 功能键：LEFT_X、RIGHT_X（GPIO低电平有效）
 * - RIGHT_X支持长按检测（阈值1秒）
 *
 * 硬件特性：
 * - 所有按键接地（低电平有效）
 * - 内部上拉电阻
 * - 按下时GPIO读取为0，释放时为1
 *
 * 使用示例：
 * @code
 * // 扫描按键
 * Key_type key = Int_key_get();
 * switch (key)
 * {
 *     case KEY_UP:
 *         // 处理UP按键
 *         break;
 *     case KEY_RIGHT_X_LONG:
 *         // 处理RIGHT_X长按
 *         break;
 * }
 * @endcode
 *
 * @note   按键扫描周期20ms
 * @note   消抖时间5ms
 * @note   函数会阻塞直到按键释放
 */

#ifndef __INT_KEY__
#define __INT_KEY__

/* ======================== 头文件引用 ======================== */

#include "main.h"           /* 主头文件（GPIO定义） */
#include "FreeRTOS.h"       /* FreeRTOS内核 */
#include "task.h"           /* FreeRTOS任务 */

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 按键类型枚举
 *
 * 定义所有可用的按键类型：
 * - KEY_NONE: 无按键按下
 * - KEY_UP/DOWN/LEFT/RIGHT: 方向键
 * - KEY_LEFT_X: 左侧功能键
 * - KEY_RIGHT_X: 右侧功能键（短按）
 * - KEY_RIGHT_X_LONG: 右侧功能键（长按>1秒）
 */
typedef enum
{
    KEY_NONE = 0,           /**< 无按键按下 */
    KEY_UP,                 /**< 上方向键 */
    KEY_DOWN,               /**< 下方向键 */
    KEY_LEFT,               /**< 左方向键 */
    KEY_RIGHT,              /**< 右方向键 */
    KEY_LEFT_X,             /**< 左侧功能键 */
    KEY_RIGHT_X,            /**< 右侧功能键（短按） */
    KEY_RIGHT_X_LONG,       /**< 右侧功能键（长按>1秒） */
} Key_type;

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  扫描并返回按键状态
 *
 * @return Key_type: KEY_NONE表示无按键按下，其他值表示具体按键
 *
 * @details 扫描优先级顺序：
 *          1. 方向键（UP、DOWN、LEFT、RIGHT）
 *          2. LEFT_X功能键
 *          3. RIGHT_X功能键（支持长按检测）
 *
 * 消抖算法：
 * 1. 检测到低电平（按键按下）
 * 2. 延时5ms（消抖）
 * 3. 再次检测，如果仍为低电平则确认按下
 * 4. 等待按键释放（防止重复触发）
 * 5. 返回按键类型
 *
 * @note   函数会阻塞直到按键释放
 * @note   如果无按键按下，立即返回KEY_NONE
 * @note   消抖时间5ms，长按阈值1000ms
 */
Key_type Int_key_get(void);

#endif /* __INT_KEY__ */
