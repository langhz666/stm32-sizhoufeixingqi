/**
 * @file    Inf_font.h
 * @brief   OLED字库数据接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件声明了OLED显示模块使用的字库数据：
 *          - ASCII字符字库（1206和1608两种尺寸）
 *          - 中文字符字库（1616尺寸）
 *          - 信号图标字库
 *          - 电池电量字库
 *
 * 字库说明：
 * - oled_asc2_1206：6x12像素ASCII字库（95个字符，从空格开始）
 * - oled_asc2_1608：8x16像素ASCII字库（95个字符，从空格开始）
 * - oled_CH_1616：16x16像素中文字符字库（半角存储）
 * - oled_CH0~CH21：分组中文字符字库
 *
 * @note   字库数据存储在Flash中（const修饰）
 * @note   字库数据由取模软件生成
 */

#ifndef __INF_FONT__
#define __INF_FONT__

/* ======================== ASCII字库声明 ======================== */

/**
 * @brief 6x12像素ASCII字库
 * @note  95个字符（从空格' '到'~'），每字符12字节
 */
extern const unsigned char oled_asc2_1206[95][12];

/**
 * @brief 8x16像素ASCII字库
 * @note  95个字符（从空格' '到'~'），每字符16字节
 */
extern const unsigned char oled_asc2_1608[95][16];

/* ======================== 中文字库声明 ======================== */

/**
 * @brief 16x16像素中文字符字库（半角存储）
 * @note  每个中文字符由两个半角字符组成
 */
extern const unsigned char oled_CH_1616[][12];

/**
 * @name 分组中文字库
 * @brief 按功能分组的中文字符字库
 * @{
 */
extern const unsigned char oled_CH0[][12];
extern const unsigned char oled_CH1[][12];
extern const unsigned char oled_CH2[][12];
extern const unsigned char oled_CH3[][12];
extern const unsigned char oled_CH4[][12];
extern const unsigned char oled_CH5[][12];
extern const unsigned char oled_CH6[][12];
extern const unsigned char oled_CH7[][12];
extern const unsigned char oled_CH8[][12];
extern const unsigned char oled_CH9[][12];

extern const unsigned char oled_CH10[][12];
extern const unsigned char oled_CH11[][12];
extern const unsigned char oled_CH12[][12];
extern const unsigned char oled_CH13[][12];
extern const unsigned char oled_CH14[][12];
extern const unsigned char oled_CH15[][12];
extern const unsigned char oled_CH16[][12];
extern const unsigned char oled_CH17[][12];
extern const unsigned char oled_CH18[][12];
extern const unsigned char oled_CH19[][12];

extern const unsigned char oled_CH20[][12];
/** @} */

#endif /* __INF_FONT__ */
