#include "App_freeRTOS_Task.h"

// STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统
// 内存管理 => C语言中的结构体通常保存在堆中  不会自动垃圾回收  => 始终使用同一个结构体 不断循环使用

// LED结构体
LED_Struct left_top_led = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
LED_Struct right_top_led = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
LED_Struct right_bottom_led = {.port = LED3_GPIO_Port, .pin = LED3_Pin};
LED_Struct left_bottom_led = {.port = LED4_GPIO_Port, .pin = LED4_Pin};

// 表示当前连接状态
Remote_State remote_state = REMOTE_DISCONNECTED;

// 表示当前的飞行状态
Flight_State flight_state = IDLE;

// 扩展获取接收的遥控数据
Remote_Data remote_data = {.thr = 0, .yaw = 500, .pit = 500, .rol = 500, .fix_height = 0, .shutdown = 0};

// 按下定高的时候 飞行的高度
uint16_t fix_height = 0;

// 电池电压 => 数组保存
uint8_t back_buff[TX_PLOAD_WIDTH] = {0};

// 电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;
// 定义任务的周期
#define POWER_TASK_PERIOD 10000

// 飞行控制任务
void flight_task(void *args);
#define FLIGHT_TASK_STACK_SIZE 128
#define FLIGHT_TASK_PRIORITY 3
TaskHandle_t flight_task_handle;
#define FLIGHT_TASK_PERIOD 6

// LED任务
void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_task_handle;
#define LED_TASK_PERIOD 100

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 4
TaskHandle_t com_task_handle;
// 任务周期
#define COM_TASK_PERIOD 10

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    // 2. 创建飞行控制任务
    xTaskCreate(flight_task, "flight_task", FLIGHT_TASK_STACK_SIZE, NULL, FLIGHT_TASK_PRIORITY, &flight_task_handle);

    // 3. 创建LED灯任务
    xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_task_handle);

    // 4. 创建通讯任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 5. 启动调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {

        // // 每10s执行一次  =>  启动电源  避免自动关机
        // vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);

        // // 启动电源
        // Int_IP5305T_start();

        // 使用直接任务通知的接收方法实现10s处理一次
        // 一直等任务通知  直到收到通知res=1  或者  超时res=0
        uint32_t res = ulTaskNotifyTake(pdTRUE, POWER_TASK_PERIOD);
        if (res != 0)
        {
            // 收到关机通知
            Int_IP5305T_shutdown();
        }
        else
        {
            // 不需要关机 => 正常执行启动
            Int_IP5305T_start();
        }
    }
}

void flight_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    // 一定要先对MPU6050执行初始化操作 => 之后才能读取数据
    App_flight_init();
    while (1)
    {
        // 1. 获根据MPU6050测量的数据  姿态解算得到欧拉角
        App_flight_get_euler_angle();

        // 2. 根据当前的欧拉角  进行PID计算控制
        App_flight_pid_process();

        // 3. 判断定高
        if (flight_state == FIX_HEIGHT)
        {
            // 才需要计算PID  => 激光测距仪的数据采集 20ms一次
            count++;
            if (count >= 4)
            {
                App_flight_fix_height_pid_process();
                count = 0;
            }
        }

        // 3. 根据PID计算的结果 对电机进行控制
        App_flight_control_motor();

        // // 4. 打印激光测距仪得到的距离值
        // uint16_t distance = Int_VL53L1X_GetDistance();
        // debug_printf("distance:%d\r\n", distance);

        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_PERIOD);
    }
}

void led_task(void *args)
{

    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    while (1)
    {
        count++;
        // 前两个灯表示连接状态
        // 1. 判断当前连接状态
        if (remote_state == REMOTE_CONNECTED)
        {
            // 点亮前两个灯
            Int_led_turn_on(&left_top_led);
            Int_led_turn_on(&right_top_led);
        }
        else if (remote_state == REMOTE_DISCONNECTED)
        {
            // 关掉前两个灯
            Int_led_turn_off(&left_top_led);
            Int_led_turn_off(&right_top_led);
        }

        // 后两个灯表示飞行状态
        // 2. 判断当前飞行状态
        if (flight_state == IDLE)
        {
            // 灯慢闪烁 => 500ms亮 500ms灭
            if (count % 5 == 0)
            {
                // 循环5次  一次是100ms  5次等于500ms
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == NORMAL)
        {
            // 灯快闪  =>  200ms亮 200ms灭
            if (count % 2 == 0)
            {
                // 循环2次  一次是100ms  2次等于200ms
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
        }
        else if (flight_state == FIX_HEIGHT)
        {
            // 后两个灯常量
            Int_led_turn_on(&left_bottom_led);
            Int_led_turn_on(&right_bottom_led);
        }
        else if (flight_state == FAIL)
        {
            // 后两个灯灭
            Int_led_turn_off(&left_bottom_led);
            Int_led_turn_off(&right_bottom_led);
        }

        // 将count计数重置
        if (count == 10)
        {
            count = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, LED_TASK_PERIOD);
    }
}

void com_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Int_bat_ADC_Init();
    while (1)
    {
        // 1. 接收数据
        uint8_t res = App_receive_data();

        // 2. 根据接收数据的返回值 处理当前飞机的连接状态
        App_process_connect_state(res);

        // 3. 处理关机命令
        if (remote_data.shutdown == 1)
        {
            // 使用Int_IP5305T_shutdown 关机  功能可以实现  但是项目结构比较奇怪
            // Int_IP5305T_shutdown();

            // 使用freeRTOS直接任务通知 => 通知电源任务 => 执行关机
            xTaskNotifyGive(power_task_handle);
        }

        // 4. 处理飞机的飞行状态  => 遇到故障状态(失联) => 会一直等待任务通知
        App_process_flight_state();

        // 5. 准备回传电池电压值
        float voltage = Int_bat_ADC_Read();
        sprintf((char *)back_buff, "%.2f", voltage);
        // debug_printf("voltage:%.2f\r\n", voltage);

        // 6ms执行一次 接收数据的时间间隔应该等于发送数据的时间间隔
        vTaskDelay(COM_TASK_PERIOD);
    }
}
