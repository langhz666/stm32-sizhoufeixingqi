/**
 * @file    Inf_OLED.h
 * @brief   SSD1306 OLED显示模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了SSD1306 OLED显示模块的公共接口：
 *          - GPIO引脚宏定义（CS/DC/RST/SDIN/SCLK）
 *          - GPIO操作宏（置位/清零）
 *          - 命令/数据标志定义
 *          - 显示函数声明
 *
 * 硬件连接（软件SPI）：
 * - CS   -> PA4  （片选，低电平有效）
 * - DC   -> PB1  （数据/命令选择）
 * - RST  -> PB0  （复位，低电平有效）
 * - SDIN -> PA7  （SPI数据输入）
 * - SCLK -> PA5  （SPI时钟）
 *
 * 显示屏参数：
 * - 驱动芯片：SSD1306
 * - 分辨率：128x64像素
 * - 颜色：单色（白色）
 * - 接口：软件SPI（GPIO位操作）
 *
 * 使用示例：
 * @code
 * OLED_Init();                              // 初始化OLED
 * OLED_Clear();                             // 清屏
 * OLED_ShowString(0, 0, "Hello", 16, 1);    // 显示字符串
 * OLED_Refresh_Gram();                      // 刷新显示
 * @endcode
 *
 * @note   所有绘制操作先写入GRAM缓冲区，调用OLED_Refresh_Gram()刷新到屏幕
 * @note   GRAM缓冲区大小：128x8字节（128x64像素，每字节8像素）
 */

#ifndef __INF_OLED__
#define __INF_OLED__

/* ======================== 头文件引用 ======================== */

#include "main.h"
#include "gpio.h"

/* ======================== GPIO引脚定义 ======================== */

/**
 * @name OLED引脚GPIO端口定义
 * @{
 */
#define OLED_CS_GPIO    GPIOA           /**< 片选引脚端口 */
#define OLED_CS_Pin     GPIO_PIN_4      /**< 片选引脚号 */

#define OLED_DC_GPIO    GPIOB           /**< 数据/命令引脚端口 */
#define OLED_DC_Pin     GPIO_PIN_1      /**< 数据/命令引脚号 */

#define OLED_RST_GPIO   GPIOB           /**< 复位引脚端口 */
#define OLED_RST_Pin    GPIO_PIN_0      /**< 复位引脚号 */

#define OLED_SDIN_GPIO  GPIOA           /**< 数据输入引脚端口 */
#define OLED_SDIN_Pin   GPIO_PIN_7      /**< 数据输入引脚号 */

#define OLED_SCLK_GPIO  GPIOA           /**< 时钟引脚端口 */
#define OLED_SCLK_Pin   GPIO_PIN_5      /**< 时钟引脚号 */
/** @} */

/* ======================== GPIO操作宏 ======================== */

/**
 * @name 片选引脚操作（CS，低电平有效）
 * @{
 */
#define OLED_CS_Clr()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)   /**< 拉低CS，选中OLED */
#define OLED_CS_Set()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)     /**< 拉高CS，释放OLED */
/** @} */

/**
 * @name 数据/命令引脚操作（DC）
 * @{
 */
#define OLED_DC_Clr()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)   /**< 拉低DC，发送命令 */
#define OLED_DC_Set()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)     /**< 拉高DC，发送数据 */
/** @} */

/**
 * @name 复位引脚操作（RST，低电平有效）
 * @{
 */
#define OLED_RST_Clr()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)   /**< 拉低RST，复位OLED */
#define OLED_RST_Set()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)     /**< 拉高RST，释放复位 */
/** @} */

/**
 * @name 数据输入引脚操作（SDIN/MOSI）
 * @{
 */
#define OLED_SDIN_Clr() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET)   /**< 拉低SDIN，发送0 */
#define OLED_SDIN_Set() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET)     /**< 拉高SDIN，发送1 */
/** @} */

/**
 * @name 时钟引脚操作（SCLK/SCK）
 * @{
 */
#define OLED_SCLK_Clr() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)   /**< 拉低SCLK */
#define OLED_SCLK_Set() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET)     /**< 拉高SCLK */
/** @} */

/* ======================== 命令/数据标志 ======================== */

#define OLED_CMD    0       /**< 命令模式标志 */
#define OLED_DATA   1       /**< 数据模式标志 */

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  初始化OLED显示屏
 * @details 配置GPIO引脚，发送SSD1306初始化命令序列
 */
void OLED_Init(void);

/**
 * @brief  向OLED写入一个字节
 * @param  dat 要写入的数据
 * @param  cmd 0=命令，1=数据
 */
void OLED_WR_Byte(uint8_t dat, uint8_t cmd);

/**
 * @brief  开启OLED显示
 */
void OLED_Display_On(void);

/**
 * @brief  关闭OLED显示
 */
void OLED_Display_Off(void);

/**
 * @brief  刷新GRAM缓冲区到OLED屏幕
 */
void OLED_Refresh_Gram(void);

/**
 * @brief  清屏（GRAM缓冲区清零并刷新）
 */
void OLED_Clear(void);

/**
 * @brief  画点
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  t 1=点亮，0=熄灭
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);

/**
 * @brief  显示单个字符
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  chr 字符（ASCII）
 * @param  size 字体大小（12或16）
 * @param  mode 0=反白显示，1=正常显示
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**
 * @brief  显示字符串
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  p 字符串指针
 * @param  size 字体大小（12或16）
 * @param  mode 0=反白显示，1=正常显示
 */
void OLED_ShowString(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size, uint8_t mode);

/**
 * @brief  显示中文字符（半角）
 * @param  x X坐标
 * @param  y Y坐标
 * @param  chr 字符索引
 * @param  size 字体大小
 * @param  mode 显示模式
 */
void OLED_ShowCH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**
 * @brief  显示中文字符（全角，占两列）
 * @param  x X坐标
 * @param  y Y坐标
 * @param  chr 字符索引
 * @param  size 字体大小
 * @param  mode 显示模式
 */
void OLED_Show_CH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

#endif /* __INF_OLED__ */
