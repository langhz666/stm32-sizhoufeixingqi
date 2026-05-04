/**
 * @file    Inf_OLED.c
 * @brief   SSD1306 OLED显示模块驱动
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块实现SSD1306 OLED显示屏的驱动：
 *          - 软件SPI位操作通信
 *          - GRAM缓冲区管理（128x8字节）
 *          - 基本绘图功能（画点、字符、字符串）
 *          - 中文字符显示
 *          - SSD1306初始化序列
 *
 * 硬件连接（软件SPI）：
 * - CS   -> PA4  （片选，低电平有效）
 * - DC   -> PB1  （数据/命令选择）
 * - RST  -> PB0  （复位，低电平有效）
 * - SDIN -> PA7  （SPI数据输入，MSB先发）
 * - SCLK -> PA5  （SPI时钟，上升沿采样）
 *
 * 通信协议：
 * - SPI模式0（CPOL=0, CPHA=0）
 * - 数据在SCLK上升沿采样
 * - MSB先发
 *
 * GRAM缓冲区：
 * - 大小：128x8字节（128x64像素）
 * - 每字节控制8个垂直像素
 * - 先写入缓冲区，再刷新到屏幕
 *
 * @note   所有绘制操作先写入GRAM缓冲区
 * @note   调用OLED_Refresh_Gram()将缓冲区刷新到屏幕
 */

#include "Inf_OLED.h"
#include "Inf_font.h"
#include "stdlib.h"
#include "gpio.h"

/* ======================== 私有变量 ======================== */

/**
 * @brief GRAM显示缓冲区
 *
 * 大小：128x8字节（128x64像素）
 * 每字节控制8个垂直像素（低位在上）
 * 索引：[x][page]，page=0-7
 */
uint8_t OLED_GRAM[128][8];

/* ======================== 私有函数 ======================== */

/**
 * @brief  刷新GRAM缓冲区到OLED屏幕
 *
 * @details 遍历8个页面（page 0-7），每页128列：
 *          1. 设置页面地址（0xB0 + page）
 *          2. 设置列地址低4位（0x00）
 *          3. 设置列地址高4位（0x10）
 *          4. 发送该页128字节数据
 *
 * @note   此函数将整个GRAM缓冲区传输到OLED
 * @note   刷新时间约1ms（128x8=1024字节）
 */
void OLED_Refresh_Gram(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);  /* 设置页地址（0~7） */
        OLED_WR_Byte(0x00, OLED_CMD);       /* 设置显示位置（列低地址） */
        OLED_WR_Byte(0x10, OLED_CMD);       /* 设置显示位置（列高地址） */
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    }
}

/**
 * @brief  向OLED写入一个字节（软件SPI）
 *
 * @param  dat 要写入的数据（8位）
 * @param  cmd 0=命令，1=数据
 *
 * @details 软件SPI时序（模式0）：
 *          1. 设置DC引脚（命令/数据选择）
 *          2. 拉低CS（选中OLED）
 *          3. 逐位发送8位数据（MSB先发）
 *             - 拉低SCLK
 *          - 根据数据位设置SDIN
 *             - 拉高SCLK（上升沿采样）
 *          4. 拉高CS（释放OLED）
 *
 * @note   SPI模式0：CPOL=0, CPHA=0
 * @note   数据在SCLK上升沿被OLED采样
 */
void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
    uint8_t i;
    if (cmd)
        OLED_DC_Set();  /* 数据模式 */
    else
        OLED_DC_Clr();  /* 命令模式 */

    OLED_CS_Clr();      /* 片选使能 */

    for (i = 0; i < 8; i++)
    {
        OLED_SCLK_Clr();                /* 时钟拉低 */
        if (dat & 0x80)
            OLED_SDIN_Set();            /* 发送1 */
        else
            OLED_SDIN_Clr();            /* 发送0 */
        OLED_SCLK_Set();                /* 时钟拉高（上升沿采样） */
        dat <<= 1;                      /* 移位，准备下一位 */
    }

    OLED_CS_Set();      /* 片选释放 */
    OLED_DC_Set();      /* 恢复数据模式 */
}

/**
 * @brief  开启OLED显示
 *
 * @details 发送命令序列：
 *          - 0x8D：使能电荷泵
 *          - 0x14：电荷泵开启
 *          - 0xAF：显示开启
 */
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);   /* 使能电荷泵 */
    OLED_WR_Byte(0X14, OLED_CMD);   /* 电荷泵开启 */
    OLED_WR_Byte(0XAF, OLED_CMD);   /* 显示开启 */
}

/**
 * @brief  关闭OLED显示
 *
 * @details 发送命令序列：
 *          - 0x8D：使能电荷泵
 *          - 0x10：电荷泵关闭
 *          - 0xAE：显示关闭
 */
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);   /* 使能电荷泵 */
    OLED_WR_Byte(0X10, OLED_CMD);   /* 电荷泵关闭 */
    OLED_WR_Byte(0XAE, OLED_CMD);   /* 显示关闭 */
}

/**
 * @brief  清屏
 *
 * @details 将GRAM缓冲区全部清零并刷新到屏幕：
 *          1. 遍历8个页面，每页128列
 *          2. 将所有字节设为0x00
 *          3. 调用OLED_Refresh_Gram()刷新
 *
 * @note   清屏后屏幕全黑（无显示内容）
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        for (n = 0; n < 128; n++)
        {
            OLED_GRAM[n][i] = 0X00;
        }
    }
    OLED_Refresh_Gram();    /* 刷新到屏幕 */
}

/**
 * @brief  画点
 *
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  t 1=点亮，0=熄灭
 *
 * @details 在GRAM缓冲区中设置/清除指定像素：
 *          1. 计算页面号：page = 7 - y/8
 *          2. 计算位号：bit = y%8
 *          3. 根据t值设置或清除对应位
 *
 * @note   坐标原点在左上角
 * @note   超出范围的坐标会被忽略
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63)
        return; /* 超出范围 */

    pos = 7 - y / 8;       /* 计算页面号 */
    bx = y % 8;            /* 计算位号 */
    temp = 1 << (7 - bx);  /* 生成位掩码 */

    if (t)
        OLED_GRAM[x][pos] |= temp;     /* 点亮像素 */
    else
        OLED_GRAM[x][pos] &= ~temp;    /* 熄灭像素 */
}

/**
 * @brief  显示单个字符
 *
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  chr 字符（ASCII）
 * @param  size 字体大小（12或16）
 * @param  mode 0=反白显示，1=正常显示
 *
 * @details 根据字体大小选择字库：
 *          - size=12：使用1206字库（6x12像素）
 *          - size=16：使用1608字库（8x16像素）
 *
 * @note   字符从空格（' '）开始，偏移量=chr-' '
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    chr = chr - ' '; /* 计算字库偏移量 */

    for (t = 0; t < size; t++)
    {
        if (size == 12)
            temp = oled_asc2_1206[chr][t]; /* 使用1206字库 */
        else
            temp = oled_asc2_1608[chr][t]; /* 使用1608字库 */

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief  显示中文字符（半角）
 *
 * @param  x X坐标
 * @param  y Y坐标
 * @param  chr 字符索引
 * @param  size 字体大小
 * @param  mode 显示模式
 *
 * @details 从中文字符字库中读取点阵数据并显示
 */
void OLED_ShowCH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;

    for (t = 0; t < size; t++)
    {
        temp = oled_CH_1616[chr][t]; /* 读取中文字符字库 */
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief  显示中文字符（全角，占两列）
 *
 * @param  x X坐标
 * @param  y Y坐标
 * @param  chr 字符索引
 * @param  size 字体大小
 * @param  mode 显示模式
 *
 * @details 全角中文字符由两个半角字符组成：
 *          - 左半部分：chr*2
 *          - 右半部分：chr*2+1
 */
void OLED_Show_CH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    OLED_ShowCH(x, y, chr * 2, size, 1);           /* 左半部分 */
    OLED_ShowCH(x + size / 2, y, chr * 2 + 1, size, 1); /* 右半部分 */
}

/**
 * @brief  显示字符串
 *
 * @param  x X坐标（0-127）
 * @param  y Y坐标（0-63）
 * @param  p 字符串指针
 * @param  size 字体大小（12或16）
 * @param  mode 0=反白显示，1=正常显示
 *
 * @details 逐个字符显示，自动换行：
 *          - X坐标超过122时自动换行
 *          - Y坐标超过58时回到原点
 *
 * @note   字符宽度：size=12时6像素，size=16时8像素
 */
void OLED_ShowString(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size, uint8_t mode)
{
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58
    while (*p != '\0')
    {
        if (x > MAX_CHAR_POSX)
        {
            x = 0;
            y += 16;
        } /* 自动换行 */
        if (y > MAX_CHAR_POSY)
        {
            y = x = 0;
        }
        OLED_ShowChar(x, y, *p, size, mode);
        x += 8;
        p++;
    }
}

/* ======================== 初始化函数 ======================== */

/**
 * @brief  初始化OLED显示屏
 *
 * @details 初始化流程：
 *          1. 配置GPIO引脚（推挽输出，高速）
 *          2. 设置初始引脚电平
 *          3. 复位OLED（RST拉低100ms后释放）
 *          4. 发送SSD1306初始化命令序列
 *
 * SSD1306初始化命令序列：
 * - 0xAE：关闭显示
 * - 0x00/0x10：设置列地址
 * - 0x40：设置起始行
 * - 0x81/0xCF：设置对比度
 * - 0xA1：设置段重映射（左右反转）
 * - 0xC0：设置COM扫描方向（上下反转）
 * - 0xA6：正常显示（非反色）
 * - 0xA8/0x3F：设置复用率（1/64）
 * - 0xD3/0x00：设置显示偏移
 * - 0xD5/0x80：设置时钟分频
 * - 0xD9/0xF1：设置预充电周期
 * - 0xDA/0x12：设置COM引脚配置
 * - 0xDB/0x30：设置VCOMH电压
 * - 0x20/0x02：设置页地址模式
 * - 0x8D/0x14：使能电荷泵
 * - 0xAF：开启显示
 *
 * @note   复位后需要等待100ms
 * @note   使用页地址模式（水平寻址）
 */
void OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 设置初始引脚电平 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);

    /* 配置PA4/PA5/PA7为推挽输出（CS/SCLK/SDIN） */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 配置PB0/PB1为推挽输出（RST/DC） */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 复位OLED */
    OLED_RST_Clr();     /* 拉低RST */
    HAL_Delay(100);     /* 等待100ms */
    OLED_RST_Set();     /* 释放RST */

    /* SSD1306初始化命令序列 */
    OLED_WR_Byte(0xAE, OLED_CMD);  /* 关闭显示 */
    OLED_WR_Byte(0x00, OLED_CMD);  /* 设置列地址低4位 */
    OLED_WR_Byte(0x10, OLED_CMD);  /* 设置列地址高4位 */
    OLED_WR_Byte(0x40, OLED_CMD);  /* 设置起始行地址 */
    OLED_WR_Byte(0x81, OLED_CMD);  /* 设置对比度控制 */
    OLED_WR_Byte(0xCF, OLED_CMD);  /* 对比度值=0xCF */
    OLED_WR_Byte(0xA1, OLED_CMD);  /* 段重映射（左右反转） */
    OLED_WR_Byte(0xC0, OLED_CMD);  /* COM扫描方向（上下反转） */
    OLED_WR_Byte(0xA6, OLED_CMD);  /* 正常显示（非反色） */
    OLED_WR_Byte(0xA8, OLED_CMD);  /* 设置复用率 */
    OLED_WR_Byte(0x3f, OLED_CMD);  /* 1/64 duty */
    OLED_WR_Byte(0xD3, OLED_CMD);  /* 设置显示偏移 */
    OLED_WR_Byte(0x00, OLED_CMD);  /* 偏移量=0 */
    OLED_WR_Byte(0xd5, OLED_CMD);  /* 设置时钟分频 */
    OLED_WR_Byte(0x80, OLED_CMD);  /* 分频值=0x80 */
    OLED_WR_Byte(0xD9, OLED_CMD);  /* 设置预充电周期 */
    OLED_WR_Byte(0xF1, OLED_CMD);  /* 预充电=15，放电=1 */
    OLED_WR_Byte(0xDA, OLED_CMD);  /* 设置COM引脚配置 */
    OLED_WR_Byte(0x12, OLED_CMD);  /* COM配置=0x12 */
    OLED_WR_Byte(0xDB, OLED_CMD);  /* 设置VCOMH电压 */
    OLED_WR_Byte(0x30, OLED_CMD);  /* VCOMH=0x30 */
    OLED_WR_Byte(0x20, OLED_CMD);  /* 设置页地址模式 */
    OLED_WR_Byte(0x02, OLED_CMD);  /* 页地址模式 */
    OLED_WR_Byte(0x8D, OLED_CMD);  /* 使能电荷泵 */
    OLED_WR_Byte(0x14, OLED_CMD);  /* 电荷泵开启 */
    OLED_Clear();                   /* 清屏 */
    OLED_WR_Byte(0xAF, OLED_CMD);  /* 开启显示 */
}
