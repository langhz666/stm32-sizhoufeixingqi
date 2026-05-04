/**
 * @file    App_freeRTOS_Task.c
 * @brief   FreeRTOS任务管理 - 创建并初始化所有系统任务
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责管理遥控器的所有FreeRTOS任务，包括：
 *          - 电源保持任务：定期触发IP5305T电源按键，防止自动关机
 *          - 通信任务：通过SI24R1无线模块与飞机进行数据交换
 *          - 按键任务：扫描按键输入并处理微调、模式切换等事件
 *          - 摇杆任务：采集4通道ADC数据并进行校准处理
 *          - 显示任务：刷新OLED屏幕显示遥控器状态信息
 *
 * @note    STM32F103C8T6仅有20KB SRAM，FreeRTOS内核约占用12KB
 *          每个任务栈大小为512字节(128*4)，需谨慎分配内存
 *
 * 任务优先级设计原则：
 * - 优先级数值越小，优先级越高
 * - 显示任务优先级最高(1)，确保UI响应流畅
 * - 电源任务优先级最低(4)，仅作保活用途
 * - 按键和摇杆任务优先级相同(2)，轮流执行
 */

#include "App_freeRTOS_Task.h"

/* ======================== 电源保持任务配置 ======================== */
/**
 * @brief 电源保持任务函数声明
 * @param args FreeRTOS任务参数（未使用）
 *
 * 该任务每10秒执行一次，通过拉低POWER_KEY引脚100ms
 * 来模拟按键按下，防止IP5305T电源芯片自动关机
 */
void power_task(void *args);

#define POWER_TASK_STACK_SIZE   128     /**< 任务栈大小：128*4 = 512字节 */
#define POWER_TASK_PRIORITY     4       /**< 任务优先级：4（最低） */
#define POWER_TASK_PERIOD       10000   /**< 任务周期：10000ms = 10秒 */

TaskHandle_t power_task_handle;         /**< 电源任务句柄，用于任务管理 */

/* ======================== 通信任务配置 ======================== */
/**
 * @brief 通信任务函数声明
 * @param args FreeRTOS任务参数（未使用）
 *
 * 该任务负责将遥控器数据打包并通过SI24R1发送到飞机
 * 使用vTaskDelay()而非vTaskDelayUntil()，因为发送时间不固定
 */
void com_task(void *args);

#define COM_TASK_STACK_SIZE     128     /**< 任务栈大小：128*4 = 512字节 */
#define COM_TASK_PRIORITY       3       /**< 任务优先级：3 */
#define COM_TASK_PERIOD         10      /**< 任务周期：10ms */

TaskHandle_t com_task_handle;           /**< 通信任务句柄 */

/* ======================== 按键扫描任务配置 ======================== */
/**
 * @brief 按键扫描任务函数声明
 * @param args FreeRTOS任务参数（未使用）
 *
 * 扫描所有GPIO按键，处理：
 * - 方向键：调节Pitch/Roll微调值（每次±10）
 * - LEFT_X键：发送关机指令
 * - RIGHT_X键：短按切换定高模式，长按(>1s)校准摇杆
 */
void key_task(void *args);

#define KEY_TASK_STACK_SIZE     128     /**< 任务栈大小：128*4 = 512字节 */
#define KEY_TASK_PRIORITY       2       /**< 任务优先级：2 */
#define KEY_TASK_PERIOD         20      /**< 任务周期：20ms（消抖周期） */

TaskHandle_t key_task_handle;           /**< 按键任务句柄 */

/* ======================== 摇杆采集任务配置 ======================== */
/**
 * @brief 摇杆采集任务函数声明
 * @param args FreeRTOS任务参数（未使用）
 *
 * 通过DMA方式采集4通道ADC数据（THR/YAW/PIT/ROL）
 * 将原始0-4095范围映射到0-1000控制范围
 * 应用校准偏移和按键微调值
 */
void joy_task(void *args);

#define JOY_TASK_STACK_SIZE     128     /**< 任务栈大小：128*4 = 512字节 */
#define JOY_TASK_PRIORITY       2       /**< 任务优先级：2（与按键任务相同） */
#define JOY_TASK_PERIOD         20      /**< 任务周期：20ms */

TaskHandle_t joy_task_handle;           /**< 摇杆任务句柄 */

/* ======================== OLED显示任务配置 ======================== */
/**
 * @brief OLED显示任务函数声明
 * @param args FreeRTOS任务参数（未使用）
 *
 * 刷新OLED屏幕内容：
 * - 第1行：标题/Logo
 * - 第2行：RF信道号 + 电池电压
 * - 第3行：THR和ROL条形图
 * - 第4行：YAW和PIT条形图
 */
void oled_task(void *args);

#define OLED_TASK_STACK_SIZE    128     /**< 任务栈大小：128*4 = 512字节 */
#define OLED_TASK_PRIORITY      1       /**< 任务优先级：1（最高） */
#define OLED_TASK_PERIOD        100     /**< 任务周期：100ms */

TaskHandle_t oled_task_handle;          /**< OLED显示任务句柄 */

/* ======================== 函数实现 ======================== */

/**
 * @brief  初始化FreeRTOS任务系统并启动调度器
 *
 * @details 按照优先级从低到高的顺序创建所有任务：
 *          1. 电源保持任务（优先级4）- 最低优先级
 *          2. 通信任务（优先级3）
 *          3. 按键扫描任务（优先级2）
 *          4. 摇杆采集任务（优先级2）
 *          5. OLED显示任务（优先级1）- 最高优先级
 *
 * @note   此函数永远不会返回 - 调度器启动后控制权交给FreeRTOS
 * @note   所有任务创建完成后才启动调度器，确保系统完整性
 *
 * @warning 如果任何任务创建失败，系统将无法正常工作
 *          可通过xTaskCreate()返回值检查是否创建成功
 */
void App_freeRTOS_start(void)
{
    /* 创建电源保持任务 - 优先级最低，仅作保活用途 */
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    /* 创建2.4G通信任务 - 处理与飞机的数据交换 */
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    /* 创建按键扫描任务 - 处理用户输入事件 */
    xTaskCreate(key_task, "key_task", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &key_task_handle);

    /* 创建摇杆采集任务 - 采集ADC数据并处理 */
    xTaskCreate(joy_task, "joy_task", JOY_TASK_STACK_SIZE, NULL, JOY_TASK_PRIORITY, &joy_task_handle);

    /* 创建OLED显示任务 - 优先级最高，确保UI响应 */
    xTaskCreate(oled_task, "oled_task", OLED_TASK_STACK_SIZE, NULL, OLED_TASK_PRIORITY, &oled_task_handle);

    /* 启动FreeRTOS调度器 - 此后控制权交给内核 */
    vTaskStartScheduler();
}

/**
 * @brief  电源管理任务
 *
 * @details 定期触发IP5305T电源按键，防止芯片进入自动关机模式。
 *          IP5305T在无负载时会在30秒后自动关机，本任务每10秒
 *          发送一个100ms的低脉冲来保持电源开启。
 *
 * @param  args FreeRTOS任务参数（未使用）
 *
 * @note   使用vTaskDelayUntil()确保精确的10秒周期
 * @note   优先级最低(4)，不影响其他任务执行
 */
void power_task(void *args)
{
    /* 获取当前节拍值作为基准时间 */
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        /* 等待下一个周期 - 使用绝对延时确保精确周期 */
        vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);

        /* 发送电源保持脉冲：拉低100ms后拉高 */
        Int_IP5305T_start();
    }
}

/**
 * @brief  摇杆采集任务
 *
 * @details 初始化ADC DMA并持续采集4通道摇杆数据：
 *          - 通过DMA自动更新adc_buff[]缓冲区
 *          - 调用App_process_joystick_data()处理原始数据
 *          - 将ADC值(0-4095)映射到控制范围(0-1000)
 *          - 应用校准偏移和按键微调
 *
 * @param  args FreeRTOS任务参数（未使用）
 *
 * @note   ADC通道映射：CH1=THR, CH6=YAW, CH2=PIT, CH3=ROL
 * @note   使用vTaskDelayUntil()确保20ms固定采样周期
 */
void joy_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* 初始化ADC DMA，启动4通道连续转换 */
    Int_joystick_init();

    while (1)
    {
        /* 采集并处理摇杆数据 */
        App_process_joystick_data();

        /* 等待下一个采样周期 */
        vTaskDelayUntil(&xLastWakeTime, JOY_TASK_PERIOD);
    }
}

/**
 * @brief  按键扫描任务
 *
 * @details 扫描所有GPIO按键并处理按键事件：
 *          - 方向键(UP/DOWN/LEFT/RIGHT)：调节Pitch/Roll微调值
 *          - LEFT_X键：设置关机标志
 *          - RIGHT_X键：短按切换定高，长按校准摇杆
 *
 * @param  args FreeRTOS任务参数（未使用）
 *
 * @note   按键消抖时间5ms，等待释放机制防止重复触发
 * @note   RIGHT_X长按阈值：1秒(1000ms)
 */
void key_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        /* 扫描并处理按键事件 */
        App_process_key_data();

        /* 等待下一个扫描周期 */
        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_PERIOD);
    }
}

/**
 * @brief  通信任务
 *
 * @details 将遥控器数据打包并通过SI24R1发送到飞机：
 *          - 帧格式：[帧头3B][数据8B][标志2B][校验4B] = 17字节
 *          - 帧头："lhz"用于设备识别
 *          - 校验：前13字节累加和，32位大端序
 *
 * @param  args FreeRTOS任务参数（未使用）
 *
 * @note   使用vTaskDelay()而非vTaskDelayUntil()，因为发送时间不固定
 * @note   发送后切换到RX模式接收飞机响应（如电池电压）
 * @note   实际周期可能因发送/接收时间而略有变化
 */
void com_task(void *args)
{
    while (1)
    {
        /* 打包数据并发送到飞机 */
        App_transmit_data();

        /* 固定延时 - 允许发送/接收时间的灵活性 */
        vTaskDelay(COM_TASK_PERIOD);
    }
}

/**
 * @brief  OLED显示任务
 *
 * @details 初始化OLED并持续刷新显示内容：
 *          - 第1行：标题/Logo
 *          - 第2行：RF信道号(3位数字) + 电池电压
 *          - 第3行：THR条形图 + ROL条形图
 *          - 第4行：YAW条形图 + PIT条形图
 *
 * @param  args FreeRTOS任务参数（未使用）
 *
 * @note   条形图以500为中位值，显示偏离中心的程度
 * @note   每个条形图段代表约83个单位(1000/12)
 * @note   使用vTaskDelayUntil()确保100ms固定刷新周期
 */
void oled_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* 初始化OLED显示模块 */
    App_display_init();

    while (1)
    {
        /* 刷新显示缓冲区并更新屏幕 */
        App_display_show();

        /* 等待下一个刷新周期 */
        vTaskDelayUntil(&xLastWakeTime, OLED_TASK_PERIOD);
    }
}
