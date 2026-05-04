/**
 * @file    App_freeRTOS_Task.c
 * @brief   FreeRTOS任务管理实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    STM32F103C8T6 SRAM 20KB，分配12KB给FreeRTOS
 *          任务栈使用静态分配，避免内存碎片
 */

#include "App_freeRTOS_Task.h"

/* ======================== LED定义 ======================== */

/** @brief 左上LED (指示遥控器连接状态) */
LED_Struct left_top_led = {.port = LED1_GPIO_Port, .pin = LED1_Pin};

/** @brief 右上LED (指示遥控器连接状态) */
LED_Struct right_top_led = {.port = LED2_GPIO_Port, .pin = LED2_Pin};

/** @brief 右下LED (指示飞行状态) */
LED_Struct right_bottom_led = {.port = LED3_GPIO_Port, .pin = LED3_Pin};

/** @brief 左下LED (指示飞行状态) */
LED_Struct left_bottom_led = {.port = LED4_GPIO_Port, .pin = LED4_Pin};

/* ======================== 全局状态变量 ======================== */

/** @brief 遥控器连接状态 */
Remote_State remote_state = REMOTE_DISCONNECTED;

/** @brief 飞行状态 */
Flight_State flight_state = IDLE;

/** @brief 遥控器数据 (默认值: 油门0，其他500) */
Remote_Data remote_data = {.thr = 0, .yaw = 500, .pit = 500, .rol = 500, .fix_height = 0, .shutdown = 0};

/** @brief 定高目标高度 */
uint16_t fix_height = 0;

/** @brief 电池电压回复缓冲区 */
uint8_t back_buff[TX_PLOAD_WIDTH] = {0};

/* ======================== 任务配置 ======================== */

/* --- 电源管理任务 --- */
void power_task(void *args);
#define POWER_TASK_STACK_SIZE   128     /**< 栈大小 128*4 = 512B */
#define POWER_TASK_PRIORITY     4       /**< 优先级 (数值越小优先级越低) */
TaskHandle_t power_task_handle;
#define POWER_TASK_PERIOD       10000   /**< 周期 10秒 */

/* --- 飞行控制任务 --- */
void flight_task(void *args);
#define FLIGHT_TASK_STACK_SIZE  128
#define FLIGHT_TASK_PRIORITY    3
TaskHandle_t flight_task_handle;
#define FLIGHT_TASK_PERIOD      6       /**< 周期 6ms (166Hz) */

/* --- LED指示任务 --- */
void led_task(void *args);
#define LED_TASK_STACK_SIZE     128
#define LED_TASK_PRIORITY       1       /**< 最低优先级 */
TaskHandle_t led_task_handle;
#define LED_TASK_PERIOD         100     /**< 周期 100ms */

/* --- 通信处理任务 --- */
void com_task(void *args);
#define COM_TASK_STACK_SIZE     128
#define COM_TASK_PRIORITY       4
TaskHandle_t com_task_handle;
#define COM_TASK_PERIOD         10      /**< 周期 10ms */

/* ======================== 任务启动 ======================== */

/**
 * @brief 启动FreeRTOS任务调度系统
 * @note  创建所有任务并启动调度器
 */
void App_freeRTOS_start(void)
{
    /* 1. 创建电源管理任务 */
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE,
                NULL, POWER_TASK_PRIORITY, &power_task_handle);

    /* 2. 创建飞行控制任务 */
    xTaskCreate(flight_task, "flight_task", FLIGHT_TASK_STACK_SIZE,
                NULL, FLIGHT_TASK_PRIORITY, &flight_task_handle);

    /* 3. 创建LED指示任务 */
    xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE,
                NULL, LED_TASK_PRIORITY, &led_task_handle);

    /* 4. 创建通信处理任务 */
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE,
                NULL, COM_TASK_PRIORITY, &com_task_handle);

    /* 5. 启动任务调度器 */
    vTaskStartScheduler();
}

/* ======================== 电源管理任务 ======================== */

/**
 * @brief 电源管理任务
 * @note  功能:
 *        - 每10秒执行一次自动开机
 *        - 收到关机通知时执行关机
 *        - 使用任务通知实现关机信号
 */
void power_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        /*
         * 等待任务通知:
         * - 收到通知: 执行关机
         * - 超时(10秒): 执行自动开机
         */
        uint32_t res = ulTaskNotifyTake(pdTRUE, POWER_TASK_PERIOD);
        if (res != 0)
        {
            /* 收到关机通知 */
            Int_IP5305T_shutdown();
        }
        else
        {
            /* 超时: 执行自动开机 */
            Int_IP5305T_start();
        }
    }
}

/* ======================== 飞行控制任务 ======================== */

/**
 * @brief 飞行控制任务 (核心任务)
 * @note  执行流程 (6ms周期):
 *        1. 读取MPU6050数据，解算欧拉角
 *        2. 计算三轴PID
 *        3. 定高PID (24ms周期)
 *        4. 电机混控输出
 */
void flight_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;

    /* 初始化飞控 (含MPU6050校准) */
    App_flight_init();

    while (1)
    {
        /* 1. 姿态解算: 读取传感器数据 -> 滤波 -> 计算欧拉角 */
        App_flight_get_euler_angle();

        /* 2. PID控制: 根据遥控器指令和当前姿态计算PID输出 */
        App_flight_pid_process();

        /* 3. 定高控制: 每24ms执行一次 (激光传感器采样周期) */
        if (flight_state == FIX_HEIGHT)
        {
            count++;
            if (count >= 4) /* 4 * 6ms = 24ms */
            {
                App_flight_fix_height_pid_process();
                count = 0;
            }
        }

        /* 4. 电机控制: 混控输出到四个电机 */
        App_flight_control_motor();

        /* 周期延时 (精确6ms) */
        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_PERIOD);
    }
}

/* ======================== LED指示任务 ======================== */

/**
 * @brief LED指示任务
 * @note  LED指示逻辑:
 *        前两个LED: 遥控器连接状态 (亮=连接, 灭=断开)
 *        后两个LED: 飞行状态
 *        - IDLE: 慢闪 (500ms亮/500ms灭)
 *        - NORMAL: 快闪 (200ms亮/200ms灭)
 *        - FIX_HEIGHT: 常亮
 *        - FAIL: 常灭
 */
void led_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;

    while (1)
    {
        count++;

        /* --- 遥控器连接状态指示 --- */
        if (remote_state == REMOTE_CONNECTED)
        {
            Int_led_turn_on(&left_top_led);
            Int_led_turn_on(&right_top_led);
        }
        else if (remote_state == REMOTE_DISCONNECTED)
        {
            Int_led_turn_off(&left_top_led);
            Int_led_turn_off(&right_top_led);
        }

        /* --- 飞行状态指示 --- */
        if (flight_state == IDLE)
        {
            /* 慢闪: 500ms周期 (5 * 100ms) */
            if (count % 5 == 0)
            {
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == NORMAL)
        {
            /* 快闪: 200ms周期 (2 * 100ms) */
            if (count % 2 == 0)
            {
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == FIX_HEIGHT)
        {
            /* 常亮 */
            Int_led_turn_on(&left_bottom_led);
            Int_led_turn_on(&right_bottom_led);
        }
        else if (flight_state == FAIL)
        {
            /* 常灭 */
            Int_led_turn_off(&left_bottom_led);
            Int_led_turn_off(&right_bottom_led);
        }

        /* 计数器归零 (10 * 100ms = 1秒) */
        if (count == 10)
        {
            count = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, LED_TASK_PERIOD);
    }
}

/* ======================== 通信处理任务 ======================== */

/**
 * @brief 通信处理任务
 * @note  执行流程 (10ms周期):
 *        1. 接收遥控数据
 *        2. 处理连接状态
 *        3. 处理关机指令
 *        4. 处理飞行状态机
 *        5. 读取电池电压并回复
 */
void com_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* 初始化电池ADC */
    Int_bat_ADC_Init();

    while (1)
    {
        /* 1. 接收并解析遥控数据 */
        uint8_t res = App_receive_data();

        /* 2. 处理遥控器连接状态 */
        App_process_connect_state(res);

        /* 3. 处理关机指令 */
        if (remote_data.shutdown == 1)
        {
            /* 通过任务通知请求电源管理任务执行关机 */
            xTaskNotifyGive(power_task_handle);
        }

        /* 4. 处理飞行状态机 */
        App_process_flight_state();

        /* 5. 读取电池电压并准备回复数据 */
        float voltage = Int_bat_ADC_Read();
        sprintf((char *)back_buff, "%.2f", voltage);

        /* 周期延时 */
        vTaskDelay(COM_TASK_PERIOD);
    }
}
