/**
 * @file    App_display.h
 * @brief   OLED显示模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了OLED显示模块的公共接口：
 *          - 显示布局常量定义
 *          - App_display_init()函数声明
 *          - App_display_show()函数声明
 *
 * 显示布局（128x64像素）：
 * - 第1行 (Y=0):  标题/Logo
 * - 第2行 (Y=14): RF信道 + 电池电压
 * - 第3行 (Y=26): THR条形图 + ROL条形图
 * - 第4行 (Y=38): YAW条形图 + PIT条形图
 *
 * @note   OLED使用SPI接口，软件模拟时序
 * @note   显示刷新周期100ms
 */

#ifndef __APP_DISPLAY__
#define __APP_DISPLAY__

/* ======================== 头文件引用 ======================== */

/* 驱动层头文件 */
#include "Inf_OLED.h"           /* OLED驱动 */
#include "Int_SI24R1.h"         /* 2.4G模块（获取CHANNEL宏） */

/* 应用层头文件 */
#include "App_process_data.h"   /* 数据处理模块（获取remote_data） */

/* ======================== 布局常量定义 ======================== */

/**
 * @name 第1行布局常量
 * @{
 */
#define LINE1_BEGIN     28      /**< 第1行起始X坐标 */
/** @} */

/**
 * @name 第2行布局常量
 * @{
 */
#define LINE2_BEGIN     5       /**< 第2行起始X坐标 */
/** @} */

/**
 * @name 第3行布局常量（THR和ROL）
 * @{
 */
#define LINE3_BEGIN     5       /**< 第3行起始X坐标 */
#define BAR1_BEGING     35      /**< 左侧条形图起始X坐标 */
#define BAR2_BEGING     47      /**< 右侧条形图起始X坐标 */
#define LINE3_BEGIN2    65      /**< 第3行右侧起始X坐标 */
#define BAR1_BEGING2    95      /**< 右侧条形图起始X坐标 */
#define BAR2_BEGING2    107     /**< 右侧条形图结束X坐标 */
/** @} */

/**
 * @name 第4行布局常量（YAW和PIT）
 * @{
 */
#define LINE4_BEGIN     5       /**< 第4行起始X坐标 */
#define LINE4_BEGIN2    65      /**< 第4行右侧起始X坐标 */
/** @} */

/**
 * @name Y轴位置常量
 * @{
 */
#define Y0              0       /**< 第1行Y坐标 */
#define Y1              14      /**< 第2行Y坐标 */
#define Y2              26      /**< 第3行Y坐标 */
#define Y3              38      /**< 第4行Y坐标 */
/** @} */

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  初始化OLED显示模块
 *
 * @details 调用OLED硬件初始化函数，包括：
 *          - GPIO引脚配置
 *          - SPI时序初始化
 *          - SSD1306控制器初始化序列
 *          - 清屏操作
 *
 * @note   此函数在oled_task开始时调用一次
 * @note   初始化完成后OLED处于关闭状态
 */
void App_display_init(void);

/**
 * @brief  刷新显示内容
 *
 * @details 渲染所有显示元素到OLED屏幕：
 *          - 第1行：标题/Logo
 *          - 第2行：RF信道号 + 电池电压
 *          - 第3行：THR和ROL条形图
 *          - 第4行：YAW和PIT条形图
 *
 * 条形图说明：
 * - 中位值为500，显示偏离中心的程度
 * - 值>500：右侧填充，左侧满格
 * - 值<500：左侧按比例填充
 *
 * @note   此函数在oled_task中每100ms调用一次
 * @note   显示内容在RAM缓冲区中绘制，最后一次性刷新
 */
void App_display_show(void);

#endif /* __APP_DISPLAY__ */
