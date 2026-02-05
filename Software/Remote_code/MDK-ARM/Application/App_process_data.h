#ifndef __APP_PROCESS_DATA__
#define __APP_PROCESS_DATA__

#include "Int_joystick.h"
#include "Int_key.h"
#include "Com_debug.h"
#include "Com_tool.h"
typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown;   // 1: 关闭  0: 不关机
    uint8_t fix_height; // 1. 切换定高和不定高 0: 不切换
} Remote_Data;

/**
 * @brief 处理按键数据 => 如果有按键按下 需要进行对应的记录
 *
 */
void App_process_key_data(void);

/**
 * @brief 处理摇杆数据  => 修正极性相位和标准值
 *
 */
void App_process_joystick_data(void);

#endif // __APP_PROCESS_DATA__
