#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_IP5305T.h"
#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "App_transmit_data.h"
#include "App_display.h"
/**
 * @brief 启动freeRTOS操作系统
 * 
 */
void App_freeRTOS_start(void);

#endif // __APP_FREERTOS_TASK__
