/**
 * @file    Int_IP5305T.c
 * @brief   IP5305T电源管理模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details IP5305T是一款集成充放电管理的电源芯片：
 *          - 内置升压转换器
 *          - 支持边充边放
 *          - 自动关机功能（无负载30秒后关机）
 *          - 按键开关机
 *
 * 本模块通过定期触发电源按键来防止自动关机：
 * - 每10秒发送一个100ms的低脉冲
 * - 模拟用户按下电源键
 * - 保持电源开启状态
 *
 * 硬件连接：
 * - POWER_KEY -> PB15（低电平有效）
 * - 按键接地，内部上拉
 * - 拉低100ms触发按键动作
 *
 * @note   必须每10秒触发一次，否则芯片自动关机
 * @note   按键脉冲宽度100ms，符合IP5305T时序要求
 */

#include "Int_IP5305T.h"

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
 *
 * @warning POWER_KEY引脚配置为推挽输出，无上拉电阻
 *          确保初始化时设置为高电平（释放状态）
 */
void Int_IP5305T_start(void)
{
    /* 步骤1：拉低POWER_KEY引脚（模拟按键按下） */
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);

    /* 步骤2：保持100ms */
    vTaskDelay(100);

    /* 步骤3：拉高POWER_KEY引脚（模拟按键释放） */
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}
