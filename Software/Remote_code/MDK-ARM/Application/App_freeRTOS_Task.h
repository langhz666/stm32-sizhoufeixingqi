/**
 * @file    App_freeRTOS_Task.h
 * @brief   FreeRTOS任务管理模块接口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本头文件定义了FreeRTOS任务管理模块的公共接口：
 *          - App_freeRTOS_start()函数声明
 *          - 所有任务相关的头文件引用
 *
 * 依赖模块：
 * - FreeRTOS: 实时操作系统内核
 * - Com_debug: 调试打印功能
 * - Int_IP5305T: 电源管理驱动
 * - Int_SI24R1: 2.4G无线模块驱动
 * - App_process_data: 数据处理模块
 * - App_transmit_data: 数据传输模块
 * - App_display: 显示控制模块
 *
 * @note   此文件仅声明App_freeRTOS_start()函数
 *         其他任务函数为内部实现，不对外暴露
 */

#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

/* ======================== 头文件引用 ======================== */

/* FreeRTOS核心头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 依赖模块头文件 */
#include "Com_debug.h"          /* 调试打印 */
#include "Int_IP5305T.h"        /* 电源管理 */
#include "Int_SI24R1.h"         /* 2.4G无线模块 */
#include "App_process_data.h"   /* 数据处理 */
#include "App_transmit_data.h"  /* 数据传输 */
#include "App_display.h"        /* 显示控制 */

/* ======================== 公共函数声明 ======================== */

/**
 * @brief  初始化FreeRTOS任务系统并启动调度器
 *
 * @details 创建所有应用任务并启动FreeRTOS调度器：
 *          - power_task: 电源保持任务
 *          - com_task: 通信任务
 *          - key_task: 按键扫描任务
 *          - joy_task: 摇杆采集任务
 *          - oled_task: OLED显示任务
 *
 * @note   此函数永远不会返回
 * @note   调用后系统由FreeRTOS内核接管
 */
void App_freeRTOS_start(void);

#endif /* __APP_FREERTOS_TASK__ */
