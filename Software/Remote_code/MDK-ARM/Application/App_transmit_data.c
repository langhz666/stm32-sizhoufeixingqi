/**
 * @file    App_transmit_data.c
 * @brief   2.4G无线数据传输模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责将遥控器数据打包并通过SI24R1发送到飞机：
 *          - 数据帧格式设计
 *          - 校验和计算
 *          - TX/RX模式切换
 *          - 飞机响应接收
 *
 * 通信协议帧格式（17字节）：
 * ┌──────┬──────┬──────┬─────┬─────┬─────┬─────┬────┬─────┬───────┐
 * │ HEAD │ HEAD │ HEAD │ THR │ YAW │ PIT │ ROL │ SD │ FIX │  SUM  │
 * │  1B  │  1B  │  1B  │ 2B  │ 2B  │ 2B  │ 2B  │ 1B │ 1B  │  4B   │
 * └──────┴──────┴──────┴─────┴─────┴─────┴─────┴────┴─────┴───────┘
 *
 * @note    帧头"lhz"用于接收端识别设备
 * @note    数据采用大端序（高字节在前）
 * @note    校验和为前13字节的累加和
 */

#include "App_transmit_data.h"

/* ======================== 全局变量 ======================== */

/**
 * @brief 遥控器数据结构（外部引用）
 *
 * 从App_process_data模块获取处理后的摇杆值和控制标志
 */
extern Remote_Data remote_data;

/**
 * @brief 发送缓冲区
 *
 * 存储待发送的17字节数据帧
 */
static uint8_t transmit_buff[TX_PLOAD_WIDTH] = {0};

/**
 * @brief 接收缓冲区
 *
 * 存储飞机回传的数据（如电池电压等信息）
 * 通过extern在App_display模块中访问
 */
uint8_t post_buff[TX_PLOAD_WIDTH] = {0};

/* ======================== 函数实现 ======================== */

/**
 * @brief  传输遥控器数据到飞机
 *
 * @details 完整的传输流程：
 *          1. 填充帧头（3字节）
 *          2. 填充摇杆数据（8字节，大端序）
 *          3. 填充控制标志（2字节，临界区保护）
 *          4. 计算校验和（4字节，大端序）
 *          5. 切换到TX模式
 *          6. 发送数据帧
 *          7. 切换到RX模式
 *          8. 接收飞机响应（如果发送成功）
 *
 * 帧头校验字节：
 * - [0] = 'l' (0x6C)
 * - [1] = 'h' (0x68)
 * - [2] = 'z' (0x7A)
 *
 * 数据字段说明：
 * - THR：油门值，0-1000，大端序
 * - YAW：偏航值，0-1000，大端序
 * - PIT：俯仰值，0-1000，大端序
 * - ROL：横滚值，0-1000，大端序
 * - SD：关机标志，1=关机，0=无操作
 * - FIX：定高标志，1=切换定高，0=无操作
 *
 * 校验和计算：
 * - 对前13字节（帧头+数据+标志）求和
 * - 结果以32位大端序存储在字节13-16
 *
 * @note   关机和定高标志读取后自动清零，防止重复触发
 * @note   使用临界区保护标志变量的读-改-写操作
 * @note   发送失败时（MAX_RT），不会接收飞机响应
 */
void App_transmit_data(void)
{
    uint32_t checksum = 0;

    /* ========== 步骤1：填充帧头 ========== */
    transmit_buff[0] = FRAME_HEAD_CHECK_1;     /* 'l' = 0x6C */
    transmit_buff[1] = FRAME_HEAD_CHECK_2;     /* 'h' = 0x68 */
    transmit_buff[2] = FRAME_HEAD_CHECK_3;     /* 'z' = 0x7A */

    /* ========== 步骤2：填充摇杆数据（大端序） ========== */

    /* THR - 油门：高8位在前，低8位在后 */
    transmit_buff[3] = (remote_data.thr >> 8) & 0xFF;
    transmit_buff[4] = remote_data.thr & 0xFF;

    /* YAW - 偏航 */
    transmit_buff[5] = (remote_data.yaw >> 8) & 0xFF;
    transmit_buff[6] = remote_data.yaw & 0xFF;

    /* PIT - 俯仰 */
    transmit_buff[7] = (remote_data.pit >> 8) & 0xFF;
    transmit_buff[8] = remote_data.pit & 0xFF;

    /* ROL - 横滚 */
    transmit_buff[9] = (remote_data.rol >> 8) & 0xFF;
    transmit_buff[10] = remote_data.rol & 0xFF;

    /* ========== 步骤3：填充控制标志（临界区保护） ========== */

    /**
     * 临界区说明：
     * - 防止在读取标志和清零之间被其他任务修改
     * - 确保每个标志只被发送一次
     */
    taskENTER_CRITICAL();

    /* 关机标志 */
    transmit_buff[11] = remote_data.shutdown;
    remote_data.shutdown = 0;           /* 读取后清零 */

    /* 定高标志 */
    transmit_buff[12] = remote_data.fix_height;
    remote_data.fix_height = 0;         /* 读取后清零 */

    taskEXIT_CRITICAL();

    /* ========== 步骤4：计算校验和 ========== */

    /**
     * 校验和计算：
     * - 对前13字节（索引0-12）求和
     * - 用于接收端验证数据完整性
     */
    for (uint8_t i = 0; i < 13; i++)
    {
        checksum += transmit_buff[i];
    }

    /* 校验和以32位大端序存储 */
    transmit_buff[13] = (checksum >> 24) & 0xFF;    /* 最高字节 */
    transmit_buff[14] = (checksum >> 16) & 0xFF;
    transmit_buff[15] = (checksum >> 8) & 0xFF;
    transmit_buff[16] = checksum & 0xFF;            /* 最低字节 */

    /* ========== 步骤5-7：发送并接收响应 ========== */

    /* 切换到TX模式 */
    Int_SI24R1_TX_Mode();

    /* 发送数据帧 */
    uint8_t tx_result = Int_SI24R1_TxPacket(transmit_buff);

    /* 切换回RX模式，准备接收飞机响应 */
    Int_SI24R1_RX_Mode();

    /* 如果发送成功，接收飞机响应 */
    if (tx_result == 0)
    {
        /**
         * 接收响应说明：
         * - 飞机收到数据后会回传状态信息
         * - 响应数据存储在post_buff[]中
         * - 包含电池电压等信息
         * - 非阻塞读取，无数据时立即返回
         */
        while (Int_SI24R1_RxPacket(post_buff) == 0)
        {
            /* 处理响应数据（如需要） */
        }
    }
}
