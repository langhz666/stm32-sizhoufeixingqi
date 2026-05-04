/**
 * @file    Int_SI24R1.c
 * @brief   SI24R1 2.4G无线通信模块驱动实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    SI24R1兼容nRF24L01+，通过SPI接口与STM32通信
 *          支持自动应答、自动重发、多通道接收等功能
 */

#include "Int_SI24R1.h"

/** @brief 发送/接收地址 (5字节，发送和接收地址相同) */
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x06, 0x1E, 0x01};

/**
 * @brief SPI读写一个字节
 * @param byte 要发送的字节
 * @return 接收到的字节
 * @note   发送和接收同时进行
 */
static uint8_t SPI_RW(uint8_t byte)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
    return rx_data;
}

/**
 * @brief 写单个寄存器
 * @param reg   寄存器地址
 * @param value 要写入的值
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value)
{
    uint8_t status;

    CS_LOW;
    status = SPI_RW(reg);
    SPI_RW(value);
    CS_HIGH;

    return status;
}

/**
 * @brief 写多个字节到寄存器
 * @param reg   寄存器地址
 * @param pBuf  数据缓冲区首地址
 * @param size  数据字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    CS_LOW;
    status = SPI_RW(reg);
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        SPI_RW(*pBuf++);
    }
    CS_HIGH;

    return status;
}

/**
 * @brief 读单个寄存器
 * @param reg   寄存器地址
 * @return 寄存器值
 */
uint8_t Int_SI24R1_Read_Reg(uint8_t reg)
{
    uint8_t value;

    CS_LOW;
    SPI_RW(reg);
    value = SPI_RW(0);
    CS_HIGH;

    return value;
}

/**
 * @brief 读多个字节从寄存器
 * @param reg   寄存器地址
 * @param pBuf  数据缓冲区首地址
 * @param size  数据字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    CS_LOW;
    status = SPI_RW(reg);
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        pBuf[byte_ctr] = SPI_RW(0);
    }
    CS_HIGH;

    return status;
}

/**
 * @brief 设置SI24R1为接收模式
 * @note  配置:
 *        - 接收通道0地址 = 发送地址 (用于自动应答)
 *        - 使能通道0自动应答
 *        - 使能通道0接收
 *        - 射频通道 = 40
 *        - 负载宽度 = 17字节
 *        - 数据速率 = 1Mbps, 发射功率 = 4dBm
 *        - 使能CRC, 16位CRC, 上电, 接收模式
 */
void Int_SI24R1_RX_Mode(void)
{
    CE_LOW;

    /* 设置接收通道0地址 (与发送地址相同) */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);

    /* 使能通道0自动应答 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);

    /* 使能通道0接收 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);

    /* 设置射频通道 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);

    /* 设置通道0负载宽度 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);

    /* 设置数据速率和发射功率 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);

    /* 配置: 使能CRC, 16位CRC, 上电, 接收模式 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0f);

    /* 清除所有中断标志 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, 0xff);

    CE_HIGH; /* 使能接收 */
}

/**
 * @brief 设置SI24R1为发送模式
 * @note  配置:
 *        - 发送地址
 *        - 接收通道0地址 = 发送地址 (用于接收应答)
 *        - 使能通道0自动应答
 *        - 使能通道0接收
 *        - 自动重发: 等待250us+86us, 最多重发10次
 *        - 射频通道 = 40
 *        - 数据速率 = 1Mbps, 发射功率 = 4dBm
 *        - 使能CRC, 16位CRC, 上电, 发送模式
 */
void Int_SI24R1_TX_Mode(void)
{
    CE_LOW;

    /* 设置发送地址 */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

    /* 设置接收通道0地址 (用于接收应答) */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);

    /* 使能通道0自动应答 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);

    /* 使能通道0接收 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);

    /* 设置自动重发: 等待250us+86us, 最多重发10次 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);

    /* 设置射频通道 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);

    /* 设置数据速率和发射功率 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);

    /* 配置: 使能CRC, 16位CRC, 上电, 发送模式 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0e);

    CE_HIGH;
}

/**
 * @brief 接收一包数据
 * @param rxbuf 接收数据缓冲区首地址
 * @return 0: 接收到数据, 1: 没有接收到数据
 * @note   读取状态寄存器判断RX_DR标志，清除标志后读取数据
 */
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
    uint8_t state;

    /* 读取状态寄存器 */
    state = Int_SI24R1_Read_Reg(STATUS);

    /* 清除RX_DR中断标志 (写1清除) */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);

    if (state & RX_DR) /* 接收到数据 */
    {
        /* 从RX FIFO读取数据 */
        Int_SI24R1_Read_Buf(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH);

        /* 清空RX FIFO */
        Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);
        return 0;
    }

    return 1; /* 没有接收到数据 */
}

/**
 * @brief 发送一包数据
 * @param txbuf 要发送的数据缓冲区
 * @return 0: 发送成功, 1: 发送失败 (达到最大重发次数)
 * @note   发送后轮询状态寄存器，等待TX_DS或MAX_RT标志
 */
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
    uint8_t state;

    CE_LOW;

    /* 写数据到TX FIFO */
    Int_SI24R1_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);

    CE_HIGH; /* 启动发送 */

    /* 轮询等待发送完成 */
    state = Int_SI24R1_Read_Reg(STATUS);
    while (((state & TX_DS) == 0) && ((state & MAX_RT) == 0))
    {
        state = Int_SI24R1_Read_Reg(STATUS);
        vTaskDelay(1);
    }

    /* 清除中断标志 */
    Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state);

    if (state & MAX_RT) /* 达到最大重发次数 */
    {
        Int_SI24R1_Write_Reg(FLUSH_TX, 0xff); /* 清空TX FIFO */
        return 1;
    }

    if (state & TX_DS) /* 发送成功 */
    {
        return 0;
    }

    return 1; /* 发送失败 */
}

/** @brief SPI通信校验缓冲区 */
uint8_t si24r1_rx_buff[5] = {0};

/**
 * @brief SI24R1硬件自检
 * @return 0: 自检成功, 1: 自检失败
 * @note   通过写入和读取发送地址验证SPI通信
 */
uint8_t Int_SI24R1_Check(void)
{
    /* 1. 先读取一次，确保SPI通信正常 */
    Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    /* 2. 写入发送地址 */
    Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

    /* 3. 读取并比较 */
    Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    for (uint8_t i = 0; i < TX_ADR_WIDTH; i++)
    {
        if (si24r1_rx_buff[i] != TX_ADDRESS[i])
        {
            return 1; /* 自检失败 */
        }
    }

    return 0; /* 自检成功 */
}

/**
 * @brief 初始化SI24R1无线模块
 * @note  初始化流程:
 *        1. 等待芯片上电稳定 (>100ms)
 *        2. 硬件自检 (验证SPI通信)
 *        3. 设置默认为接收模式
 */
void Int_SI24R1_Init(void)
{
    /* 1. 等待芯片上电稳定 */
    HAL_Delay(200);

    /* 2. 硬件自检 */
    while (Int_SI24R1_Check() == 1)
    {
        HAL_Delay(10); /* 自检失败，延时后重试 */
    }

    /* 3. 设置默认为接收模式 */
    Int_SI24R1_RX_Mode();
    debug_printf("SI24R1 Init Success!\r\n");
}
