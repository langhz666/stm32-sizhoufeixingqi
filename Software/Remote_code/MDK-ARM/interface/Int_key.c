/**
 * @file    Int_key.c
 * @brief   按键扫描与处理模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责扫描所有GPIO按键，实现：
 *          - 按键消抖（5ms延时）
 *          - 等待释放机制（防止重复触发）
 *          - 长按检测（RIGHT_X按键，阈值1秒）
 *
 * 按键布局：
 * - 方向键：UP、DOWN、LEFT、RIGHT（GPIO低电平有效）
 * - 功能键：LEFT_X、RIGHT_X（GPIO低电平有效）
 *
 * 硬件特性：
 * - 所有按键接地（低电平有效）
 * - 内部上拉电阻
 * - 按下时GPIO读取为0，释放时为1
 *
 * @note   按键扫描周期20ms，消抖时间5ms
 * @note   等待释放机制会阻塞任务直到按键释放
 * @note   RIGHT_X长按阈值：1000ms（1秒）
 */

#include "Int_key.h"

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
 * RIGHT_X长按检测：
 * 1. 记录按下开始时间
 * 2. 等待按键释放
 * 3. 计算按下持续时间
 * 4. 如果>1秒返回KEY_RIGHT_X_LONG
 * 5. 否则返回KEY_RIGHT_X
 *
 * @note   函数会阻塞直到按键释放
 * @note   如果无按键按下，立即返回KEY_NONE
 */
Key_type Int_key_get(void)
{
    /* ========== 扫描UP按键 ========== */
    if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
    {
        /* 步骤1：检测到低电平 */
        vTaskDelay(5);  /* 步骤2：消抖延时5ms */

        /* 步骤3：再次确认 */
        if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
        {
            /* 步骤4：等待按键释放 */
            while (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);  /* 1ms延时，释放CPU */
            }

            /* 步骤5：返回按键类型 */
            return KEY_UP;
        }
    }
    /* ========== 扫描DOWN按键 ========== */
    else if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_DOWN;
        }
    }
    /* ========== 扫描LEFT按键 ========== */
    else if (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port, KEY_LEFT_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port, KEY_LEFT_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port, KEY_LEFT_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT;
        }
    }
    /* ========== 扫描RIGHT按键 ========== */
    else if (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_RIGHT;
        }
    }
    /* ========== 扫描LEFT_X按键 ========== */
    else if (HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port, KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port, KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port, KEY_LEFT_X_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT_X;
        }
    }
    /* ========== 扫描RIGHT_X按键（支持长按检测） ========== */
    else if (HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port, KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
    {
        /* 记录按下开始时间 */
        TickType_t press_start = xTaskGetTickCount();

        vTaskDelay(5);  /* 消抖延时 */
        if (HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port, KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
        {
            /* 等待按键释放 */
            while (HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port, KEY_RIGHT_X_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);  /* 1ms延时，释放CPU */
            }

            /* 计算按下持续时间 */
            TickType_t press_duration = xTaskGetTickCount() - press_start;

            /* 判断长按阈值（1秒 = 1000ms） */
            if (press_duration > 1000)
            {
                return KEY_RIGHT_X_LONG;    /* 长按 */
            }
            else
            {
                return KEY_RIGHT_X;         /* 短按 */
            }
        }
    }

    /* ========== 无按键按下 ========== */
    return KEY_NONE;
}
