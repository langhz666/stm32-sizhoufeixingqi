/**
 * @file    Int_SI24R1.c
 * @brief   SI24R1 2.4G无线模块驱动
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details SI24R1是nRF24L01+兼容的2.4GHz无线收发芯片。
 *          通过SPI接口与STM32通信，支持自动应答和自动重传。
 *
 * 主要特性：
 * - 工作频率：2.4GHz ISM频段
 * - 数据速率：1Mbps/2Mbps
 * - 发射功率：0/-6/-12/-18dBm
 * - 工作电压：1.9-3.6V
 * - SPI接口：最高10MHz
 *
 * 本驱动配置：
 * - 信道：40（2440MHz）
 * - 数据速率：1Mbps
 * - 发射功率：4dBm
 * - CRC：16位
 * - 自动应答：开启（管道0）
 * - 自动重传：10次，250us间隔
 *
 * 硬件连接：
 * - SPI1_SCK  -> PB3
 * - SPI1_MISO -> PB4
 * - SPI1_MOSI -> PB5
 * - SPI1_NSS  -> PA15（软件控制）
 * - SI_EN     -> PB7（芯片使能）
 *
 * @note   上电后需要等待>100ms稳定时间
 * @note   首次使用需验证SPI通信是否正常
 */

#include "Int_SI24R1.h"

/* ======================== 地址配置 ======================== */

/**
 * @brief 发送地址（5字节）
 *
 * 接收端必须配置相同的地址才能接收数据
 * 地址格式：{0x0A, 0x01, 0x06, 0x1E, 0x01}
 */
static uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x06, 0x1E, 0x01};

/* ======================== 底层SPI函数 ======================== */

/**
 * @brief  SPI单字节传输
 *
 * @param  byte 要发送的字节
 * @return 接收到的字节
 *
 * @details 全双工SPI传输：
 *          - 发送byte的同时接收对方数据
 *          - 使用HAL_SPI_TransmitReceive()实现
 *          - 超时时间1000ms
 *
 * @note   SPI模式0：CPOL=0, CPHA=0
 * @note   数据位序：MSB先发
 */
static uint8_t SPI_RW(uint8_t byte)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
    return rx_data;
}

/* ======================== 寄存器读写函数 ======================== */

/**
 * @brief  写单个寄存器
 *
 * @param  reg 寄存器地址（使用 SI24R1_WRITE_REG + reg_addr 格式）
 * @param  value 要写入的值
 * @return 状态寄存器值
 *
 * @details 写入流程：
 *          1. 拉低CS片选
 *          2. 发送寄存器地址（含写命令位）
 *          3. 发送数据值
 *          4. 拉高CS释放
 *
 * @note   SI24R1_WRITE_REG = 0x20，与寄存器地址相加得到写命令
 */
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value)
{
    uint8_t status;

    CS_LOW;                     /* 片选使能 */
    status = SPI_RW(reg);      /* 发送寄存器地址，读取状态 */
    SPI_RW(value);              /* 写入数据 */
    CS_HIGH;                    /* 释放片选 */

    return status;
}

/**
 * @brief  写多个字节到寄存器
 *
 * @param  reg 寄存器地址
 * @param  pBuf 数据缓冲区指针
 * @param  size 数据字节数
 * @return 状态寄存器值
 *
 * @details 连续写入流程：
 *          1. 拉低CS片选
 *          2. 发送寄存器地址
 *          3. 循环发送size个字节
 *          4. 拉高CS释放
 *
 * @note   用于写入TX地址（5字节）或TX/RX数据包
 */
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    CS_LOW;                         /* 片选使能 */
    status = SPI_RW(reg);          /* 发送寄存器地址 */

    /* 连续发送数据 */
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        SPI_RW(*pBuf++);
    }

    CS_HIGH;                        /* 释放片选 */

    return status;
}

/**
 * @brief  读单个寄存器
 *
 * @param  reg 寄存器地址（使用 SI24R1_READ_REG + reg_addr 格式）
 * @return 寄存器值
 *
 * @details 读取流程：
 *          1. 拉低CS片选
 *          2. 发送寄存器地址（含读命令位）
 *          3. 发送空字节(0x00)同时读取数据
 *          4. 拉高CS释放
 *
 * @note   SI24R1_READ_REG = 0x00，与寄存器地址相加得到读命令
 */
uint8_t Int_SI24R1_Read_Reg(uint8_t reg)
{
    uint8_t value;

    CS_LOW;                     /* 片选使能 */
    SPI_RW(reg);                /* 发送寄存器地址 */
    value = SPI_RW(0);          /* 读取数据 */
    CS_HIGH;                    /* 释放片选 */

    return value;
}

/**
 * @brief  读多个字节从寄存器
 *
 * @param  reg 寄存器地址
 * @param  pBuf 接收缓冲区指针
 * @param  size 读取字节数
 * @return 状态寄存器值
 *
 * @details 连续读取流程：
 *          1. 拉低CS片选
 *          2. 发送寄存器地址
 *          3. 循环读取size个字节
 *          4. 拉高CS释放
 *
 * @note   用于读取RX地址（5字节）或RX数据包
 */
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    CS_LOW;                         /* 片选使能 */
    status = SPI_RW(reg);          /* 发送寄存器地址 */

    /* 连续读取数据 */
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        pBuf[byte_ctr] = SPI_RW(0);
    }

    CS_HIGH;                        /* 释放片选 */

    return status;
}

/* ======================== 模式配置函数 ======================== */

/**
 * @brief  配置SI24R1为接收模式
 *
 * @details RX模式配置：
 *          - 管道0地址：与发送地址相同（用于自动应答）
 *          - 自动应答：开启（管道0）
 *          - 接收管道：开启（管道0）
 *          - RF信道：40（2440MHz）
 *          - 有效数据宽度：17字节
 *          - 数据速率：1Mbps
 *          - 发射功率：4dBm
 *          - CRC：16位
 *          - 上电：是
 *          - 模式：接收
 *
 * @note   配置完成后CE拉高，开始监听空中数据
 * @note   接收到数据后RX_DR标志置位，产生中断
 */
void Int_SI24R1_RX_Mode(void)
{
    CE_LOW;     /* CE拉低，进入配置模式 */

    /* 设置管道0接收地址（与发送地址相同，用于自动应答） */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);

    /* 使能管道0自动应答 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);

    /* 使能管道0接收 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);

    /* 设置RF信道 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);

    /* 设置有效数据宽度（与发送端相同） */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);

    /* 设置数据速率和发射功率 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);

    /* 配置寄存器：CRC使能、16位CRC、上电、接收模式 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0f);

    /* 清除所有中断标志 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, 0xff);

    CE_HIGH;    /* CE拉高，开始接收 */
}

/**
 * @brief  配置SI24R1为发送模式
 *
 * @details TX模式配置：
 *          - 发送地址：TX_ADDRESS
 *          - 管道0接收地址：与发送地址相同（用于自动应答）
 *          - 自动应答：开启（管道0）
 *          - 自动重传：10次，间隔250us+86us
 *          - RF信道：40（2440MHz）
 *          - 数据速率：1Mbps
 *          - 发射功率：4dBm
 *          - CRC：16位
 *          - 上电：是
 *          - 模式：发送
 *
 * @note   配置完成后CE拉高，触发数据发送
 * @note   发送完成后TX_DS标志置位，或达到最大重传次数MAX_RT置位
 */
void Int_SI24R1_TX_Mode(void)
{
    CE_LOW;     /* CE拉低，进入配置模式 */

    /* 设置发送地址 */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

    /* 设置管道0接收地址（与发送地址相同，用于自动应答） */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);

    /* 使能管道0自动应答 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);

    /* 使能管道0接收 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);

    /* 设置自动重传：10次，间隔250us+86us */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);

    /* 设置RF信道 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);

    /* 设置数据速率和发射功率 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);

    /* 配置寄存器：CRC使能、16位CRC、上电、发送模式 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0e);

    CE_HIGH;    /* CE拉高，触发发送 */
}

/* ======================== 数据收发函数 ======================== */

/**
 * @brief  接收数据包
 *
 * @param  rxbuf 接收缓冲区指针（>= TX_PLOAD_WIDTH字节）
 * @return 0: 接收到数据，1: 无数据
 *
 * @details 接收流程：
 *          1. 读取STATUS寄存器
 *          2. 清除中断标志（写1清除）
 *          3. 检查RX_DR标志
 *          4. 如果有数据：读取RX FIFO并清除
 *          5. 返回接收状态
 *
 * @note   RX_DR标志在读取STATUS后自动清除
 * @note   读取数据后需清除RX FIFO
 */
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
    uint8_t state;

    /* 读取状态寄存器 */
    state = Int_SI24R1_Read_Reg(STATUS);

    /* 清除中断标志（写1清除） */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);

    /* 检查RX_DR标志 - 数据接收完成 */
    if (state & RX_DR)
    {
        /* 从RX FIFO读取数据 */
        Int_SI24R1_Read_Buf(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH);

        /* 清除RX FIFO */
        Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);

        return 0;   /* 接收成功 */
    }

    return 1;   /* 无数据 */
}

/**
 * @brief  发送数据包
 *
 * @param  txbuf 发送缓冲区指针（>= TX_PLOAD_WIDTH字节）
 * @return 0: 发送成功，1: 发送失败（达到最大重传次数）
 *
 * @details 发送流程：
 *          1. CE拉低，进入待机模式
 *          2. 写入数据到TX FIFO
 *          3. CE拉高，触发发送
 *          4. 轮询STATUS寄存器等待发送完成
 *          5. 检查TX_DS（发送成功）或MAX_RT（达到最大重传）
 *          6. 清除中断标志
 *          7. 如果MAX_RT，清除TX FIFO
 *
 * @note   使用轮询方式等待发送完成（非中断方式）
 * @note   每次轮询间隔1ms，释放CPU给其他任务
 * @note   发送失败时返回1，调用者需处理重发逻辑
 */
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
    uint8_t state;

    /* CE拉低，进入待机模式 */
    CE_LOW;

    /* 写入数据到TX FIFO */
    Int_SI24R1_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);

    /* CE拉高，触发发送 */
    CE_HIGH;

    /**
     * 轮询等待发送完成：
     * - TX_DS: 数据发送成功
     * - MAX_RT: 达到最大重传次数
     * - 两者都未置位则继续等待
     */
    state = Int_SI24R1_Read_Reg(STATUS);
    while (((state & TX_DS) == 0) && ((state & MAX_RT) == 0))
    {
        state = Int_SI24R1_Read_Reg(STATUS);
        vTaskDelay(1);  /* 延时1ms，释放CPU */
    }

    /* 清除中断标志 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);

    /* 检查是否达到最大重传次数 */
    if (state & MAX_RT)
    {
        /* 清除TX FIFO */
        Int_SI24R1_Write_Reg(FLUSH_TX, 0xff);
        return 1;   /* 发送失败 */
    }

    /* 检查是否发送成功 */
    if (state & TX_DS)
    {
        return 0;   /* 发送成功 */
    }

    return 1;   /* 其他情况视为失败 */
}

/* ======================== 初始化函数 ======================== */

/**
 * @brief SPI通信验证缓冲区
 */
static uint8_t si24r1_rx_buff[5] = {0};

/**
 * @brief  验证SI24R1 SPI通信
 *
 * @return 0: 通信正常，1: 通信失败
 *
 * @details 验证流程：
 *          1. 读取当前TX地址（确保SPI就绪）
 *          2. 写入测试地址到TX寄存器
 *          3. 读回并比较
 *          4. 逐字节检查是否匹配
 *
 * @note   首次读取是dummy read，确保SPI通信链路正常
 * @note   如果验证失败，可能是硬件连接问题或芯片损坏
 */
uint8_t Int_SI24R1_Check(void)
{
    /* 读取当前TX地址（dummy read，确保SPI就绪） */
    Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    /* 写入测试地址 */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

    /* 读回验证 */
    Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    /* 逐字节比较 */
    for (uint8_t i = 0; i < TX_ADR_WIDTH; i++)
    {
        if (si24r1_rx_buff[i] != TX_ADDRESS[i])
        {
            return 1;   /* 不匹配，通信失败 */
        }
    }

    return 0;   /* 通信正常 */
}

/**
 * @brief  初始化SI24R1模块
 *
 * @details 初始化流程：
 *          1. 上电延时200ms（SI24R1需要>100ms稳定时间）
 *          2. 验证SPI通信（每10ms重试一次）
 *          3. 设置默认接收模式
 *
 * @note   初始化过程是阻塞的，直到通信验证成功
 * @note   如果硬件连接有问题，会卡在验证循环中
 * @note   初始化成功后打印调试信息
 */
void Int_SI24R1_Init(void)
{
    /* 上电延时：SI24R1需要>100ms稳定时间 */
    HAL_Delay(200);

    /* 验证SPI通信（失败则每10ms重试） */
    while (Int_SI24R1_Check() == 1)
    {
        HAL_Delay(10);
    }

    /* 设置默认接收模式 */
    Int_SI24R1_RX_Mode();

    debug_printf("SI24R1 Init Success!\r\n");
}
