/**
 * @file    App_freeRTOS_Task.h
 * @brief   FreeRTOS任务管理模块
 * @author  langhz666
 * @date    2025-09-27
 * @note    创建并管理四个系统任务:
 *          1. 电源管理任务 (power_task)
 *          2. 飞行控制任务 (flight_task)
 *          3. LED指示任务 (led_task)
 *          4. 通信处理任务 (com_task)
 */

#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Com_config.h"
#include "Int_IP5305T.h"
#include "Int_motor.h"
#include "Int_led.h"
#include "Int_SI24R1.h"
#include "Int_bat_ADC.h"
#include "App_receive_data.h"
#include "App_flight.h"

/**
 * @brief 启动FreeRTOS任务调度系统
 * @note  创建所有任务并启动调度器
 *        此函数不会返回
 */
void App_freeRTOS_start(void);

#endif /* __APP_FREERTOS_TASK__ */
