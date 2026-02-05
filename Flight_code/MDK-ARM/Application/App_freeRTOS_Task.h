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
 * @brief 启动freeRTOS操作系统
 * 
 */
void App_freeRTOS_start(void);

#endif // __APP_FREERTOS_TASK__
