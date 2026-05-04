/**
 * @file    Int_SI24R1.h
 * @brief   SI24R1 2.4G无线模块驱动接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details SI24R1是nRF24L01+兼容的2.4GHz无线收发芯片。
 *          本头文件定义了SI24R1驱动的所有公共接口。
 *
 * 主要功能：
 * - 寄存器读写
 * - TX/RX模式切换
 * - 数据包收发
 * - 模块初始化
 *
 * 硬件连接：
 * - SPI1_SCK  -> PB3
 * - SPI1_MISO -> PB4
 * - SPI1_MOSI -> PB5
 * - SPI1_NSS  -> PA15（软件控制）
 * - SI_EN     -> PB7（芯片使能）
 *
 * 使用示例：
 * @code
 * // 初始化
 * Int_SI24R1_Init();
 *
 * // 发送数据
 * uint8_t tx_buf[17] = {...};
 * Int_SI24R1_TX_Mode();
 * Int_SI24R1_TxPacket(tx_buf);
 *
 * // 接收数据
 * uint8_t rx_buf[17] = {0};
 * Int_SI24R1_RX_Mode();
 * if (Int_SI24R1_RxPacket(rx_buf) == 0)
 * {
 *     // 处理接收到的数据
 * }
 * @endcode
 */

#ifndef __INT_SI24R1_H__
#define __INT_SI24R1_H__

/* ======================== 头文件引用 ======================== */

#include "spi.h"            /* SPI驱动 */
#include "Com_debug.h"      /* 调试打印 */
#include "FreeRTOS.h"       /* FreeRTOS内核 */
#include "task.h"           /* FreeRTOS任务 */

/* ======================== GPIO控制宏 ======================== */

/**
 * @name SPI片选控制
 * @{
 */
#define CS_LOW      HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_RESET)   /**< 拉低CS，选中芯片 */
#define CS_HIGH     HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET)     /**< 拉高CS，释放芯片 */
/** @} */

/**
 * @name 芯片使能控制
 * @{
 */
#define CE_LOW      HAL_GPIO_WritePin(SI_EN_GPIO_Port, SI_EN_Pin, GPIO_PIN_RESET)         /**< 拉低CE，进入配置模式 */
#define CE_HIGH     HAL_GPIO_WritePin(SI_EN_GPIO_Port, SI_EN_Pin, GPIO_PIN_SET)           /**< 拉高CE，启动收发 */
/** @} */

/* ======================== RF配置参数 ======================== */

/**
 * @name RF参数定义
 * @{
 */
#define CHANNEL         40      /**< RF信道：0-125，频率 = 2400 + CHANNEL (MHz) */
#define TX_ADR_WIDTH    5       /**< TX/RX地址宽度（字节） */
#define TX_PLOAD_WIDTH  17      /**< 有效载荷宽度（字节） */
/** @} */

/* ======================== SI24R1 SPI命令 ======================== */

/**
 * @name SPI命令定义
 * @{
 */
#define SI24R1_READ_REG     0x00    /**< 读寄存器命令 */
#define SI24R1_WRITE_REG    0x20    /**< 写寄存器命令 */
#define RD_RX_PLOAD         0x61    /**< 读RX有效载荷 */
#define WR_TX_PLOAD         0xA0    /**< 写TX有效载荷 */
#define FLUSH_TX            0xE1    /**< 清除TX FIFO */
#define FLUSH_RX            0xE2    /**< 清除RX FIFO */
#define REUSE_TX_PL         0xE3    /**< 重用上次TX有效载荷 */
#define NOP                 0xFF    /**< 空操作（用于读取状态） */
/** @} */

/* ======================== SI24R1寄存器地址 ======================== */

/**
 * @name 寄存器地址定义
 * @{
 */
#define CONFIG          0x00    /**< 配置寄存器 */
#define EN_AA           0x01    /**< 自动应答使能 */
#define EN_RXADDR       0x02    /**< RX地址使能 */
#define SETUP_AW        0x03    /**< 地址宽度设置 */
#define SETUP_RETR      0x04    /**< 自动重传设置 */
#define RF_CH           0x05    /**< RF信道 */
#define RF_SETUP        0x06    /**< RF设置 */
#define STATUS          0x07    /**< 状态寄存器 */
#define OBSERVE_TX      0x08    /**< 发送观测 */
#define RSSI            0x09    /**< 接收信号强度 */
#define RX_ADDR_P0      0x0A    /**< RX管道0地址 */
#define RX_ADDR_P1      0x0B    /**< RX管道1地址 */
#define RX_ADDR_P2      0x0C    /**< RX管道2地址 */
#define RX_ADDR_P3      0x0D    /**< RX管道3地址 */
#define RX_ADDR_P4      0x0E    /**< RX管道4地址 */
#define RX_ADDR_P5      0x0F    /**< RX管道5地址 */
#define TX_ADDR         0x10    /**< TX地址 */
#define RX_PW_P0        0x11    /**< RX管道0有效载荷宽度 */
#define RX_PW_P1        0x12    /**< RX管道1有效载荷宽度 */
#define RX_PW_P2        0x13    /**< RX管道2有效载荷宽度 */
#define RX_PW_P3        0x14    /**< RX管道3有效载荷宽度 */
#define RX_PW_P4        0x15    /**< RX管道4有效载荷宽度 */
#define RX_PW_P5        0x16    /**< RX管道5有效载荷宽度 */
#define FIFO_STATUS     0x17    /**< FIFO状态寄存器 */
/** @} */

/* ======================== STATUS寄存器位定义 ======================== */

/**
 * @name STATUS寄存器位
 * @{
 */
#define RX_DR           0x40    /**< 数据就绪中断标志 */
#define TX_DS           0x20    /**< 数据发送完成中断标志 */
#define MAX_RT          0x10    /**< 达到最大重传次数中断标志 */
/** @} */

/* ======================== API函数声明 ======================== */

/**
 * @brief  写单个寄存器
 * @param  reg 寄存器地址（使用 SI24R1_WRITE_REG + reg_addr 格式）
 * @param  value 要写入的值
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value);

/**
 * @brief  写多个字节到寄存器
 * @param  reg 寄存器地址
 * @param  pBuf 数据缓冲区指针
 * @param  size 数据字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size);

/**
 * @brief  读单个寄存器
 * @param  reg 寄存器地址（使用 SI24R1_READ_REG + reg_addr 格式）
 * @return 寄存器值
 */
uint8_t Int_SI24R1_Read_Reg(uint8_t reg);

/**
 * @brief  读多个字节从寄存器
 * @param  reg 寄存器地址
 * @param  pBuf 接收缓冲区指针
 * @param  size 读取字节数
 * @return 状态寄存器值
 */
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size);

/**
 * @brief  配置SI24R1为接收模式
 * @details 配置参数：管道0地址、自动应答、RF信道40、1Mbps、16位CRC
 */
void Int_SI24R1_RX_Mode(void);

/**
 * @brief  配置SI24R1为发送模式
 * @details 配置参数：发送地址、自动重传10次、RF信道40、1Mbps、16位CRC
 */
void Int_SI24R1_TX_Mode(void);

/**
 * @brief  接收数据包
 * @param  rxbuf 接收缓冲区指针（>= TX_PLOAD_WIDTH字节）
 * @return 0: 接收到数据，1: 无数据
 */
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf);

/**
 * @brief  发送数据包
 * @param  txbuf 发送缓冲区指针（>= TX_PLOAD_WIDTH字节）
 * @return 0: 发送成功，1: 发送失败（达到最大重传次数）
 */
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf);

/**
 * @brief  初始化SI24R1模块
 * @details 初始化流程：上电延时200ms -> 验证SPI通信 -> 设置默认接收模式
 */
void Int_SI24R1_Init(void);

#endif /* __INT_SI24R1_H__ */
