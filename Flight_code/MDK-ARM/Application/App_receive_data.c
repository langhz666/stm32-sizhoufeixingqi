/*
 * @Author: langhz666 3204498297@qq.com
 * @Date: 2025-09-27 16:30:00
 * @LastEditors: langhz666 3204498297@qq.com
 * @LastEditTime: 2026-02-04 23:54:11
 * @FilePath: \MDK-ARM\Application\App_receive_data.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "App_receive_data.h"

extern Remote_Data remote_data;

uint8_t rx_buff[TX_PLOAD_WIDTH] = {0};

// 遥控连接状态
extern Remote_State remote_state;
// 飞行状态
extern Flight_State flight_state;

// 油门解锁状态值
Thr_state thr_state = FREE;
// MAX状态的进入时间
uint32_t max_enter_time = 0;
// MIN状态的进入时间
uint32_t min_enter_time = 0;
// 重试次数
uint8_t retry_count = 0;

// 按下定高之后的飞行高度
extern uint16_t fix_height;
extern uint8_t back_buff[TX_PLOAD_WIDTH];

/**
 * @brief 接收遥控器发送的遥控数据 => 解析为结构体
 *
 * @return uint8_t 0:校验通过 是正常的数据 1:没收到数据 或者 校验失败
 */
uint8_t App_receive_data(void)
{
    memset(rx_buff, 0, TX_PLOAD_WIDTH);
    uint8_t res = Int_SI24R1_RxPacket(rx_buff);
    if (res == 0)
    {
        // 收到遥控数据 => 准备回传
        Int_SI24R1_TX_Mode();

        uint16_t count = 500;
        // 回传也要严格按照时序 => 从接收成功遥控数据开始发送 一直到发送完成
        while (Int_SI24R1_TxPacket(back_buff) == 1 && count--)
        {
        }
        Int_SI24R1_RX_Mode();
    }

    if (strlen((char *)rx_buff) == 0)
    {
        return 1;
    }

    // 1. 帧头校验
    if (rx_buff[0] != FRAME_HEAD_CHECK_1 || rx_buff[1] != FRAME_HEAD_CHECK_2 || rx_buff[2] != FRAME_HEAD_CHECK_3)
    {
        return 1;
    }

    // 2. 帧尾校验
    uint32_t sum = 0;
    uint32_t sum_receive = 0;

    for (uint8_t i = 0; i < 13; i++)
    {
        sum += rx_buff[i];
    }
    // 高位在前
    sum_receive = rx_buff[13] << 24 | rx_buff[14] << 16 | rx_buff[15] << 8 | rx_buff[16];

    if (sum != sum_receive)
    {
        return 1;
    }

    // 3. 保存数据
    remote_data.thr = (rx_buff[3] << 8) | rx_buff[4];
    remote_data.yaw = (rx_buff[5] << 8) | rx_buff[6];
    remote_data.pit = (rx_buff[7] << 8) | rx_buff[8];
    remote_data.rol = (rx_buff[9] << 8) | rx_buff[10];
    remote_data.shutdown = rx_buff[11];
    remote_data.fix_height = rx_buff[12];

    // debug_printf(":%d,%d,%d,%d,%d,%d\n", remote_data.thr, remote_data.yaw, remote_data.pit, remote_data.rol, remote_data.shutdown, remote_data.fix_height);
    return 0;
}

/**
 * @brief 处理连接状态的状态
 *
 * @param res 上一次接收数据的返回值
 */
void App_process_connect_state(uint8_t res)
{
    if (res == 0)
    {
        // 接收数据成功一次 即为连接成功
        // 此处使用的全局变量 只有当前一个地方会修改 LED灯控任务当中是读取使用
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else if (res == 1)
    {
        // 接收数据失败 即为
        retry_count++;
        if (retry_count >= MAX_RETRY_TIMES)
        {
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}

/**
 * @brief 处理解锁逻辑
 *
 * @return uint8_t 0: 解锁成功 1: 解锁失败
 */
static uint8_t App_process_unlock(void)
{
    // 1. 考虑安全问题 =>  解锁完成的最终状态应该是油门为0
    switch (thr_state)
    {
    case FREE:
        if (remote_data.thr >= 900)
        {
            // 2. 进入max状态
            thr_state = MAX;
            // freeRTOS操作系统中以ms为单位计数的时间
            max_enter_time = xTaskGetTickCount();
        }

        break;
    case MAX:
        // 3. 持续的时间应该是离开的时间减去进入的时间
        if (remote_data.thr < 900)
        {
            if (xTaskGetTickCount() - max_enter_time >= 1000)
            {
                // 4. 油门保持最高状态超过1s => 进入leave_max状态
                thr_state = LEAVE_MAX;
            }
            else
            {
                // 5. 油门保持最高状态时间小于1s => 退回到free 重新解锁
                thr_state = FREE;
            }
        }

        break;
    case LEAVE_MAX:
        if (remote_data.thr <= 100)
        {
            // 6. 油门回到0  进入min状态
            thr_state = MIN;
            min_enter_time = xTaskGetTickCount();
        }

        break;
    case MIN:
        // 7. 每次判断当前已经保持了多久
        if (xTaskGetTickCount() - min_enter_time <= 1000)
        {
            // 还不够1s
            if (remote_data.thr > 100)
            {
                thr_state = FREE;
            }
        }
        else
        {
            // 已经保持够1s => 解锁完成
            thr_state = UNLOCK;
        }

        break;
    case UNLOCK:
        /* code */
        break;
    default:
        break;
    }

    if (thr_state == UNLOCK)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief 处理飞机的飞行状态
 *
 */
void App_process_flight_state(void)
{
    // 使用状态机逻辑实现
    // 1. 轮询调用判断当前所处的状态
    switch (flight_state)
    {
    case IDLE:
        // 2. 只需要编写指向其他状态的代码即可
        if (App_process_unlock() == 0)
        {
            flight_state = NORMAL;
            // 每一次解锁成功  需要将解锁状态重置
            thr_state = FREE;
        }

        break;
    case NORMAL:
        // 3. 判断进入定高
        if (remote_data.fix_height == 1)
        {
            flight_state = FIX_HEIGHT;
            remote_data.fix_height = 0;

            // 记录下当前目标的高度
            fix_height = Int_VL53L1X_GetDistance();
        }
        // 4. 判断进入故障失联状态
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
        }

        break;
    case FIX_HEIGHT:
        // 5. 取消定高
        if (remote_data.fix_height == 1)
        {
            flight_state = NORMAL;
            remote_data.fix_height = 0;
        }
        // 6. 判断故障
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
        }
        break;
    case FAIL:
        // 7.处理失联故障  缓慢停止电机
        // 等待故障处理完成 => 一直等处理完成 不会出现超时
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        flight_state = IDLE;
        break;
    default:
        break;
    }
}
