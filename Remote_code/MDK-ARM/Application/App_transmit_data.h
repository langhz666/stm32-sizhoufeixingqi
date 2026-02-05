/*
 * @Author: langhz666 3204498297@qq.com
 * @Date: 2025-09-27 16:30:01
 * @LastEditors: langhz666 3204498297@qq.com
 * @LastEditTime: 2026-02-04 23:55:05
 * @FilePath: \MDK-ARM\Application\App_transmit_data.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __APP_TRANSMIT_DATA__
#define __APP_TRANSMIT_DATA__

#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "FreeRTOS.h"
#include "task.h"
// 定义帧头校验的值
#define FRAME_HEAD_CHECK_1 'l'
#define FRAME_HEAD_CHECK_2 'h'
#define FRAME_HEAD_CHECK_3 'z'

/**
 * @brief 自动切换SI24R1的模式 => 将采集完成的遥控数据打包发送到飞机
 * 
 */
void App_transmit_data(void);


#endif // __APP_TRANSMIT_DATA__
