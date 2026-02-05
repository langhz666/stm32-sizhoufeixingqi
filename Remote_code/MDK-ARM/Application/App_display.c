#include "App_display.h"
extern Remote_Data remote_data;
// 回传的电压值
extern uint8_t post_buff[TX_PLOAD_WIDTH];
void App_display_show_bar(uint8_t x, uint8_t y, uint8_t count)
{
    if (count < 13)
    {
        OLED_Show_CH(x, y, 12 + count, 12, 1);
    }
}

/**
 * @brief 初始化显示模块
 *
 */
void App_display_init(void)
{
    OLED_Init();
}

/**
 * @brief 循环执行刷写屏幕
 *
 */
void App_display_show(void)
{
    // 将显示内容写入到缓存
    // (1) X坐标  (2) Y坐标 (3)字符串 (4) 字体大小 (5) 模式(字白底黑)
    // OLED_ShowString(0, 0, "Hello World", 12, 1);

    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_Show_CH(LINE1_BEGIN + 12 * i, Y0, i, 12, 1);
    }
    
    
    OLED_ShowString(0,0,"123",12,1);
    OLED_Show_CH(LINE1_BEGIN + 60+32-2, Y0, 11, 12, 1);

    // 第二行 => 展示2.4G通信的信道
    uint8_t buff[3] = {0};
    sprintf((char *)buff, "%03d", CHANNEL);
    OLED_ShowString(LINE2_BEGIN, Y1, buff, 12, 1);

    // 第二行 => 右边显示回传的电压值  => 推算出剩余的电量
    OLED_ShowString(LINE2_BEGIN + 64, Y1, "V:  ", 12, 1);
    OLED_ShowString(LINE2_BEGIN + 88, Y1, post_buff, 12, 1);

    uint8_t count = 0;
    // 第三行 => 展示遥控数据  THR  ROL
    OLED_ShowString(LINE3_BEGIN, Y2, "THR:", 12, 1);
    if (remote_data.thr > 500)
    {
        count = (remote_data.thr - 500) / 41;
        // 前一个汉字字符 涂满
        App_display_show_bar(BAR1_BEGING, Y2, 12);
        App_display_show_bar(BAR2_BEGING, Y2, count);
    }
    else
    {
        count = (remote_data.thr) / 41;
        // 只图前一个字符  后一个图空白
        App_display_show_bar(BAR1_BEGING, Y2, count);
        App_display_show_bar(BAR2_BEGING, Y2, 0);
    }

    OLED_ShowString(LINE3_BEGIN2, Y2, "ROL:", 12, 1);

    if (remote_data.rol > 500)
    {
        count = (remote_data.rol - 500) / 41;
        // 前一个汉字字符 涂满
        App_display_show_bar(BAR1_BEGING2, Y2, 12);
        App_display_show_bar(BAR2_BEGING2, Y2, count);
    }
    else
    {
        count = (remote_data.rol) / 41;
        // 只图前一个字符  后一个图空白
        App_display_show_bar(BAR1_BEGING2, Y2, count);
        App_display_show_bar(BAR2_BEGING2, Y2, 0);
    }

    // 第四行 =>  YAW PIT
    OLED_ShowString(LINE4_BEGIN, Y3, "YAW:", 12, 1);
    if (remote_data.yaw > 500)
    {
        count = (remote_data.yaw - 500) / 41;
        // 前一个汉字字符 涂满
        App_display_show_bar(BAR1_BEGING, Y3, 12);
        App_display_show_bar(BAR2_BEGING, Y3, count);
    }
    else
    {
        count = (remote_data.yaw) / 41;
        // 只图前一个字符  后一个图空白
        App_display_show_bar(BAR1_BEGING, Y3, count);
        App_display_show_bar(BAR2_BEGING, Y3, 0);
    }

    OLED_ShowString(LINE4_BEGIN2, Y3, "PIT:", 12, 1);

    if (remote_data.pit > 500)
    {
        count = (remote_data.pit - 500) / 41;
        // 前一个汉字字符 涂满
        App_display_show_bar(BAR1_BEGING2, Y3, 12);
        App_display_show_bar(BAR2_BEGING2, Y3, count);
    }
    else
    {
        count = (remote_data.pit) / 41;
        // 只图前一个字符  后一个图空白
        App_display_show_bar(BAR1_BEGING2, Y3, count);
        App_display_show_bar(BAR2_BEGING2, Y3, 0);
    }

    // 调用刷写才能真的显示
    OLED_Refresh_Gram();
}
