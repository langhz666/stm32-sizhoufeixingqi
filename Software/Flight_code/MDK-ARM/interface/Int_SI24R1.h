/**
 * @file    Int_SI24R1.h
 * @brief   SI24R1 2.4G无线通信模块驱动
 * @author  langhz666
 * @date    2025-09-27
 * @note    SI24R1兼容nRF24L01+，通过SPI接口与STM32通信
 *          用于接收遥控器数据和发送电池电压回传
 */

#ifndef __nRF24L01P__
#define __nRF24L01P__

#include "spi.h"
#include "Com_debug.h"
#include "freeRTOS.h"
#include "task.h"

/* ======================== GPIO控制宏 ======================== */

/** @brief 拉低片选信号 (选中SPI设备) */
#define CS_LOW  HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_RESET);

/** @brief 拉高片选信号 (释放SPI设备) */
#define CS_HIGH HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET);

/** @brief 拉低使能信号 (禁用SI24R1) */
#define CE_LOW  HAL_GPIO_WritePin(SI_EN_GPIO_Port, SI_EN_Pin, GPIO_PIN_RESET);

/** @brief 拉高使能信号 (启用SI24R1) */
#define CE_HIGH HAL_GPIO_WritePin(SI_EN_GPIO_Port, SI_EN_Pin, GPIO_PIN_SET);

/* ======================== 通信参数配置 ======================== */

/** @brief 射频通道号 (0-125) */
#define CHANNEL         40

/** @brief 发送/接收地址宽度 (字节) */
#define TX_ADR_WIDTH    5

/** @brief 有效数据负载宽度 (字节) */
#define TX_PLOAD_WIDTH  17

/* ======================== SI24R1 SPI命令 ======================== */

#define SI24R1_READ_REG     0x00    /**< 读寄存器命令 */
#define SI24R1_WRITE_REG    0x20    /**< 写寄存器命令 */
#define RD_RX_PLOAD         0x61    /**< 读RX FIFO数据命令 */
#define WR_TX_PLOAD         0xA0    /**< 写TX FIFO数据命令 */
#define FLUSH_TX            0xE1    /**< 清空TX FIFO命令 */
#define FLUSH_RX            0xE2    /**< 清空RX FIFO命令 */
#define REUSE_TX_PL         0xE3    /**< 重用TX FIFO数据命令 */
#define NOP                 0xFF    /**< 空操作 (可用于读状态寄存器) */

/* ======================== SI24R1寄存器地址 ======================== */

#define CONFIG      0x00    /**< 配置寄存器 */
#define EN_AA       0x01    /**< 自动应答使能寄存器 */
#define EN_RXADDR   0x02    /**< 接收通道使能寄存器 */
#define SETUP_AW    0x03    /**< 地址宽度设置寄存器 */
#define SETUP_RETR  0x04    /**< 自动重发设置寄存器 */
#define RF_CH       0x05    /**< 射频通道寄存器 */
#define RF_SETUP    0x06    /**< 射频设置寄存器 */
#define STATUS      0x07    /**< 状态寄存器 */
#define OBSERVE_TX  0x08    /**< 发送观测寄存器 */
#define RSSI        0x09    /**< 信号强度指示寄存器 */

/* 接收通道地址寄存器 */
#define RX_ADDR_P0  0x0A    /**< 接收通道0地址 */
#define RX_ADDR_P1  0x0B    /**< 接收通道1地址 */
#define RX_ADDR_P2  0x0C    /**< 接收通道2地址 */
#define RX_ADDR_P3  0x0D    /**< 接收通道3地址 */
#define RX_ADDR_P4  0x0E    /**< 接收通道4地址 */
#define RX_ADDR_P5  0x0F    /**< 接收通道5地址 */

/* 发送地址寄存器 */
#define TX_ADDR     0x10    /**< 发送地址 */

/* 接收通道负载宽度寄存器 */
#define RX_PW_P0    0x11    /**< 接收通道0负载宽度 */
#define RX_PW_P1    0x12    /**< 接收通道1负载宽度 */
#define RX_PW_P2    0x13    /**< 接收通道2负载宽度 */
#define RX_PW_P3    0x14    /**< 接收通道3负载宽度 */
#define RX_PW_P4    0x15    /**< 接收通道4负载宽度 */
#define RX_PW_P5    0x16    /**< 接收通道5负载宽度 */

#define FIFO_STATUS 0x17    /**< FIFO状态寄存器 */

/* ======================== 状态寄存器标志位 ======================== */

#define RX_DR   0x40    /**< 接收数据就绪标志 */
#define TX_DS   0x20    /**< 发送数据成功标志 */
#define MAX_RT  0x10    /**< 达到最大重发次数标志 */

/* ======================== 函数声明 ======================== */

/**
 * @brief 写单个寄存器
 * @param reg   寄存器地址 (格式: SI24R1_WRITE_REG | reg)
 * @param value 要写入的值
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value);

/**
 * @brief 写多个字节到寄存器
 * @param reg   寄存器地址 (格式: SI24R1_WRITE_REG | reg)
 * @param pBuf  数据缓冲区首地址
 * @param size  数据字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size);

/**
 * @brief 读单个寄存器
 * @param reg   寄存器地址 (格式: SI24R1_READ_REG | reg)
 * @return 寄存器值
 */
uint8_t Int_SI24R1_Read_Reg(uint8_t reg);

/**
 * @brief 读多个字节从寄存器
 * @param reg   寄存器地址 (格式: SI24R1_READ_REG | reg)
 * @param pBuf  数据缓冲区首地址
 * @param size  数据字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size);

/**
 * @brief 设置SI24R1为接收模式
 * @note  配置接收通道0地址、使能自动应答、设置射频通道等
 */
void Int_SI24R1_RX_Mode(void);

/**
 * @brief 设置SI24R1为发送模式
 * @note  配置发送地址、接收通道0地址(用于应答)、使能自动应答等
 */
void Int_SI24R1_TX_Mode(void);

/**
 * @brief 接收一包数据
 * @param rxbuf 接收数据缓冲区首地址
 * @return 0: 接收到数据, 1: 没有接收到数据
 * @note   硬件自动将数据保存到FIFO，通过状态标志位判断是否有数据
 */
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf);

/**
 * @brief 发送一包数据
 * @param txbuf 要发送的数据缓冲区
 * @return 0: 发送成功, 1: 发送失败
 */
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf);

/**
 * @brief 初始化SI24R1无线模块
 * @note  包括硬件自检、配置验证，默认设置为接收模式
 */
void Int_SI24R1_Init(void);

#endif /* __nRF24L01P__ */
