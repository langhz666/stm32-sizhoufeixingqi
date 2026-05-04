/**
 * @file    App_display.c
 * @brief   OLED显示模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责在0.96寸OLED屏幕上显示遥控器状态信息：
 *          - 标题/Logo
 *          - RF信道号和电池电压
 *          - 摇杆值条形图
 *
 * 显示布局（128x64像素）：
 * ┌────────────────────────────────────────┐
 * │ 第1行 (Y=0):  标题/Logo               │
 * │ 第2行 (Y=14): 信道:040    电压:V:xx    │
 * │ 第3行 (Y=26): THR:[====] ROL:[====]   │
 * │ 第4行 (Y=38): YAW:[====] PIT:[====]   │
 * └────────────────────────────────────────┘
 *
 * 条形图说明：
 * - 中位值为500，显示偏离中心的程度
 * - 值>500：右侧填充，左侧满格
 * - 值<500：左侧按比例填充
 * - 每个条形图段约83个单位(1000/12)
 *
 * @note    OLED使用SPI接口，软件模拟时序
 * @note    显示刷新周期100ms，确保UI流畅
 */

#include "App_display.h"

/* ======================== 外部变量引用 ======================== */

/**
 * @brief 遥控器数据结构
 *
 * 从App_process_data模块获取处理后的摇杆值
 */
extern Remote_Data remote_data;

/**
 * @brief 飞机响应数据缓冲区
 *
 * 从App_transmit_data模块获取飞机回传的电池电压等信息
 */
extern uint8_t post_buff[TX_PLOAD_WIDTH];

/* ======================== 内部函数 ======================== */

/**
 * @brief  绘制条形图段
 *
 * @param  x X坐标（像素）
 * @param  y Y坐标（像素）
 * @param  count 填充级别（0-12，0=空，12=满）
 *
 * @details 使用自定义OLED字符绘制条形图：
 *          - 字符索引12-24对应0-12级填充
 *          - 每个字符宽12像素
 *          - 用于显示摇杆偏离中心的程度
 *
 * @note   count值应小于13，否则不绘制
 */
static void App_display_show_bar(uint8_t x, uint8_t y, uint8_t count)
{
    if (count < 13)
    {
        /* 显示自定义字符：12+count为字符索引 */
        OLED_Show_CH(x, y, 12 + count, 12, 1);
    }
}

/* ======================== 公共函数实现 ======================== */

/**
 * @brief  初始化OLED显示模块
 *
 * @details 调用OLED硬件初始化函数，包括：
 *          - GPIO引脚配置
 *          - SPI时序初始化
 *          - SSD1306控制器初始化序列
 *          - 清屏操作
 *
 * @note   初始化完成后OLED处于关闭状态，需要调用OLED_Display_On()
 */
void App_display_init(void)
{
    OLED_Init();
}

/**
 * @brief  刷新显示内容
 *
 * @details 渲染所有显示元素到OLED屏幕：
 *
 *          第1行（Y=0）：标题/Logo
 *          - 显示6个中文字符作为标题
 *          - 显示"123"字符串
 *          - 显示第11号字符
 *
 *          第2行（Y=14）：RF信道 + 电池电压
 *          - 左侧：RF信道号（3位数字，如"040"）
 *          - 右侧："V:" + 电压值字符串
 *
 *          第3行（Y=26）：THR和ROL条形图
 *          - 左侧：THR条形图（4个字符位置）
 *          - 右侧：ROL条形图（4个字符位置）
 *
 *          第4行（Y=38）：YAW和PIT条形图
 *          - 左侧：YAW条形图（4个字符位置）
 *          - 右侧：PIT条形图（4个字符位置）
 *
 * 条形图计算逻辑：
 * - 值>500：左侧满格(12)，右侧=(值-500)/41
 * - 值<500：左侧=值/41，右侧空(0)
 * - 每段约83单位(1000/12≈83)
 *
 * @note   函数执行完毕后调用OLED_Refresh_Gram()更新屏幕
 * @note   显示内容在RAM缓冲区中绘制，最后一次性刷新
 */
void App_display_show(void)
{
    uint8_t count = 0;

    /* ========== 第1行：标题/Logo ========== */

    /* 显示6个中文字符作为标题 */
    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_Show_CH(LINE1_BEGIN + 12 * i, Y0, i, 12, 1);
    }

    /* 显示"123"字符串 */
    OLED_ShowString(0, 0, "123", 12, 1);

    /* 显示第11号字符 */
    OLED_Show_CH(LINE1_BEGIN + 60 + 32 - 2, Y0, 11, 12, 1);

    /* ========== 第2行：RF信道 + 电池电压 ========== */

    /* RF信道号（3位数字） */
    uint8_t channel_str[3] = {0};
    sprintf((char *)channel_str, "%03d", CHANNEL);
    OLED_ShowString(LINE2_BEGIN, Y1, channel_str, 12, 1);

    /* 电池电压显示 */
    OLED_ShowString(LINE2_BEGIN + 64, Y1, "V:  ", 12, 1);
    OLED_ShowString(LINE2_BEGIN + 88, Y1, post_buff, 12, 1);

    /* ========== 第3行：THR + ROL 条形图 ========== */

    /* THR条形图（左侧） */
    OLED_ShowString(LINE3_BEGIN, Y2, "THR:", 12, 1);
    if (remote_data.thr > 500)
    {
        /* 值>500：左侧满格，右侧按比例 */
        count = (remote_data.thr - 500) / 41;
        App_display_show_bar(BAR1_BEGING, Y2, 12);         /* 左侧满格 */
        App_display_show_bar(BAR2_BEGING, Y2, count);      /* 右侧按比例 */
    }
    else
    {
        /* 值<500：左侧按比例，右侧空 */
        count = remote_data.thr / 41;
        App_display_show_bar(BAR1_BEGING, Y2, count);      /* 左侧按比例 */
        App_display_show_bar(BAR2_BEGING, Y2, 0);          /* 右侧空 */
    }

    /* ROL条形图（右侧） */
    OLED_ShowString(LINE3_BEGIN2, Y2, "ROL:", 12, 1);
    if (remote_data.rol > 500)
    {
        count = (remote_data.rol - 500) / 41;
        App_display_show_bar(BAR1_BEGING2, Y2, 12);
        App_display_show_bar(BAR2_BEGING2, Y2, count);
    }
    else
    {
        count = remote_data.rol / 41;
        App_display_show_bar(BAR1_BEGING2, Y2, count);
        App_display_show_bar(BAR2_BEGING2, Y2, 0);
    }

    /* ========== 第4行：YAW + PIT 条形图 ========== */

    /* YAW条形图（左侧） */
    OLED_ShowString(LINE4_BEGIN, Y3, "YAW:", 12, 1);
    if (remote_data.yaw > 500)
    {
        count = (remote_data.yaw - 500) / 41;
        App_display_show_bar(BAR1_BEGING, Y3, 12);
        App_display_show_bar(BAR2_BEGING, Y3, count);
    }
    else
    {
        count = remote_data.yaw / 41;
        App_display_show_bar(BAR1_BEGING, Y3, count);
        App_display_show_bar(BAR2_BEGING, Y3, 0);
    }

    /* PIT条形图（右侧） */
    OLED_ShowString(LINE4_BEGIN2, Y3, "PIT:", 12, 1);
    if (remote_data.pit > 500)
    {
        count = (remote_data.pit - 500) / 41;
        App_display_show_bar(BAR1_BEGING2, Y3, 12);
        App_display_show_bar(BAR2_BEGING2, Y3, count);
    }
    else
    {
        count = remote_data.pit / 41;
        App_display_show_bar(BAR1_BEGING2, Y3, count);
        App_display_show_bar(BAR2_BEGING2, Y3, 0);
    }

    /* ========== 更新OLED显示 ========== */

    /* 将RAM缓冲区内容刷新到OLED屏幕 */
    OLED_Refresh_Gram();
}
