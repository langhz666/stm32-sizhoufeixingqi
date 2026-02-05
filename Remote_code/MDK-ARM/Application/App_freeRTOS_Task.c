#include "App_freeRTOS_Task.h"

// STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统
// 电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;
#define POWER_TASK_PERIOD 10000

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t com_task_handle;
// 任务周期
#define COM_TASK_PERIOD 10

// 按键任务
void key_task(void *args);
#define KEY_TASK_STACK_SIZE 128
#define KEY_TASK_PRIORITY 2
TaskHandle_t key_task_handle;
// 任务周期
#define KEY_TASK_PERIOD 20

// 摇杆任务
void joy_task(void *args);
#define JOY_TASK_STACK_SIZE 128
#define JOY_TASK_PRIORITY 2
TaskHandle_t joy_task_handle;
// 任务周期
#define JOY_TASK_PERIOD 20

// 屏幕任务
void oled_task(void *args);
#define OLED_TASK_STACK_SIZE 128
#define OLED_TASK_PRIORITY 1
TaskHandle_t oled_task_handle;
#define OLED_TASK_PERIOD 100

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    // 2. 创建通讯任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 3. 按键任务
    xTaskCreate(key_task, "key_task", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &key_task_handle);

    // 4. 摇杆任务
    xTaskCreate(joy_task, "joy_task", JOY_TASK_STACK_SIZE, NULL, JOY_TASK_PRIORITY, &joy_task_handle);

    // 5. 屏幕任务
    xTaskCreate(oled_task, "oled_task", OLED_TASK_STACK_SIZE, NULL, OLED_TASK_PRIORITY, &oled_task_handle);
    // 6. 启动调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {

        // 每10s执行一次  =>  启动电源  避免自动关机
        vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);

        // 启动电源
        Int_IP5305T_start();
    }
}

void joy_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // 1. 初始化摇杆ADC
    Int_joystick_init();
    while (1)
    {
        // 统一的处理方式
        App_process_joystick_data();
        vTaskDelayUntil(&xLastWakeTime, JOY_TASK_PERIOD);
    }
}

void key_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        // 统一的处理方式
        App_process_key_data();
        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_PERIOD);
    }
}

void com_task(void *args)
{
    
    while (1)
    {
        // 将打包完成的数据发送到飞机
        App_transmit_data();
        // 6ms执行一次  =>  vTaskDelayUntil()算上函数运行的时间 总计延迟6ms
        // 比如此次发送是在1ms的位置  重试发送延迟2ms =>  下一次发送还是1ms(7ms) 接收方还是没有启动
        // 在第一次匹配上完成收发之后 两端一起延迟一个时间 再次收发
        vTaskDelay(COM_TASK_PERIOD);
    }
}

void oled_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    App_display_init();

    while (1)
    {
        App_display_show();

        vTaskDelayUntil(&xLastWakeTime, OLED_TASK_PERIOD);
    }
}
