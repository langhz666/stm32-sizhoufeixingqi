/**
 * @file    App_receive_data.h
 * @brief   遥控数据接收与处理模块
 * @author  langhz666
 * @date    2025-09-27
 * @note    接收SI24R1无线数据，解析遥控指令，处理飞行状态机
 *          数据帧格式: 帧头(3B) + 油门(2B) + 偏航(2B) + 俯仰(2B) + 横滚(2B) + 关机(1B) + 定高(1B) + 校验(4B)
 */

#ifndef __APP_RECEIVE_DATA__
#define __APP_RECEIVE_DATA__

#include "Int_SI24R1.h"
#include "Com_config.h"
#include "Int_VL53L1X.h"

/* ======================== 协议定义 ======================== */

/** @brief 帧头校验字节 */
#define FRAME_HEAD_CHECK_1 'l'
#define FRAME_HEAD_CHECK_2 'h'
#define FRAME_HEAD_CHECK_3 'z'

/** @brief 最大重试次数 (超过则认为遥控器断连) */
#define MAX_RETRY_TIMES 10

/* ======================== 函数声明 ======================== */

/**
 * @brief 接收并解析遥控数据
 * @return 0: 解析成功, 1: 无数据或校验失败
 * @note   数据帧校验: 帧头校验 + 校验和校验
 */
uint8_t App_receive_data(void);

/**
 * @brief 处理遥控器连接状态
 * @param res 上一次接收数据的返回值
 * @note  连续MAX_RETRY_TIMES次接收失败则认为断连
 */
void App_process_connect_state(uint8_t res);

/**
 * @brief 处理飞行状态机
 * @note  状态转换: IDLE -> NORMAL -> FIX_HEIGHT -> FAIL -> IDLE
 */
void App_process_flight_state(void);

#endif /* __APP_RECEIVE_DATA__ */
