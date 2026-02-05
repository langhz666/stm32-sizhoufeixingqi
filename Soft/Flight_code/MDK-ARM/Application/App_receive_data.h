#ifndef __APP_RECEIVE_DATA__
#define __APP_RECEIVE_DATA__

#include "Int_SI24R1.h"
#include "Com_config.h"
#include "Int_VL53L1X.h"
// 定义帧头校验的值
#define FRAME_HEAD_CHECK_1 'l'
#define FRAME_HEAD_CHECK_2 'h'
#define FRAME_HEAD_CHECK_3 'z'

// 最大重试次数
#define MAX_RETRY_TIMES 10

/**
 * @brief 接收遥控器发送的遥控数据 => 解析为结构体
 * 
 * @return uint8_t 0:校验通过 是正常的数据 1:没收到数据 或者 校验失败
 */
uint8_t App_receive_data(void);


/**
 * @brief 处理连接状态的状态
 * 
 * @param res 上一次接收数据的返回值
 */
void App_process_connect_state(uint8_t res);



/**
 * @brief 处理飞机的飞行状态
 * 
 */
void App_process_flight_state(void);

#endif // __APP_RECEIVE_DATA__
