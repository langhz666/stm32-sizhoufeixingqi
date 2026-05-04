/**
 * @file    App_receive_data.c
 * @brief   遥控数据接收与处理实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    实现遥控数据接收、校验、解析和飞行状态机控制
 */

#include "App_receive_data.h"

/* 外部变量声明 */
extern Remote_Data remote_data;

/** @brief 接收缓冲区 */
uint8_t rx_buff[TX_PLOAD_WIDTH] = {0};

/** @brief 遥控器连接状态 */
extern Remote_State remote_state;

/** @brief 飞行状态 */
extern Flight_State flight_state;

/* ======================== 解锁状态机变量 ======================== */

/** @brief 油门解锁状态 */
Thr_state thr_state = FREE;

/** @brief 进入MAX状态的时间戳 */
uint32_t max_enter_time = 0;

/** @brief 进入MIN状态的时间戳 */
uint32_t min_enter_time = 0;

/** @brief 接收失败重试计数 */
uint8_t retry_count = 0;

/* 外部变量 */
extern uint16_t fix_height;
extern uint8_t back_buff[TX_PLOAD_WIDTH];

/* ======================== 数据接收与解析 ======================== */

/**
 * @brief 接收并解析遥控数据
 * @return 0: 解析成功, 1: 无数据或校验失败
 * @note   数据帧格式 (17字节):
 *         [0-2]  帧头 'l','h','z'
 *         [3-4]  油门 (高字节在前)
 *         [5-6]  偏航 (高字节在前)
 *         [7-8]  俯仰 (高字节在前)
 *         [9-10] 横滚 (高字节在前)
 *         [11]   关机标志
 *         [12]   定高标志
 *         [13-16] 校验和 (前13字节之和)
 */
uint8_t App_receive_data(void)
{
    /* 清空接收缓冲区 */
    memset(rx_buff, 0, TX_PLOAD_WIDTH);

    /* 接收SI24R1数据 */
    uint8_t res = Int_SI24R1_RxPacket(rx_buff);
    if (res == 0)
    {
        /* 收到数据后，切换到发送模式回复电池电压 */
        Int_SI24R1_TX_Mode();

        uint16_t count = 500;
        /* 发送回复数据 (带超时) */
        while (Int_SI24R1_TxPacket(back_buff) == 1 && count--)
        {
        }

        /* 切回接收模式 */
        Int_SI24R1_RX_Mode();
    }

    /* 检查是否收到数据 */
    if (strlen((char *)rx_buff) == 0)
    {
        return 1;
    }

    /* 1. 帧头校验 */
    if (rx_buff[0] != FRAME_HEAD_CHECK_1 ||
        rx_buff[1] != FRAME_HEAD_CHECK_2 ||
        rx_buff[2] != FRAME_HEAD_CHECK_3)
    {
        return 1;
    }

    /* 2. 校验和验证 */
    uint32_t sum = 0;
    uint32_t sum_receive = 0;

    /* 计算前13字节的校验和 */
    for (uint8_t i = 0; i < 13; i++)
    {
        sum += rx_buff[i];
    }

    /* 从数据帧中提取校验和 (大端序) */
    sum_receive = rx_buff[13] << 24 | rx_buff[14] << 16 | rx_buff[15] << 8 | rx_buff[16];

    if (sum != sum_receive)
    {
        return 1;
    }

    /* 3. 解析遥控数据 */
    remote_data.thr = (rx_buff[3] << 8) | rx_buff[4];      /* 油门 */
    remote_data.yaw = (rx_buff[5] << 8) | rx_buff[6];      /* 偏航 */
    remote_data.pit = (rx_buff[7] << 8) | rx_buff[8];      /* 俯仰 */
    remote_data.rol = (rx_buff[9] << 8) | rx_buff[10];     /* 横滚 */
    remote_data.shutdown = rx_buff[11];                     /* 关机标志 */
    remote_data.fix_height = rx_buff[12];                   /* 定高标志 */

    return 0;
}

/* ======================== 连接状态处理 ======================== */

/**
 * @brief 处理遥控器连接状态
 * @param res 上一次接收数据的返回值
 * @note  res=0: 接收成功，重置重试计数
 *        res=1: 接收失败，累加重试计数
 *        连续MAX_RETRY_TIMES次失败则认为断连
 */
void App_process_connect_state(uint8_t res)
{
    if (res == 0)
    {
        /* 接收成功: 标记为已连接 */
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else if (res == 1)
    {
        /* 接收失败: 累加重试计数 */
        retry_count++;
        if (retry_count >= MAX_RETRY_TIMES)
        {
            /* 超过最大重试次数: 标记为断连 */
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}

/* ======================== 解锁逻辑 ======================== */

/**
 * @brief 油门解锁处理
 * @return 0: 解锁成功, 1: 未解锁
 * @note   解锁流程 (安全保护):
 *         1. 油门推到最高 (>900)
 *         2. 保持1秒以上
 *         3. 油门拉到最低 (<100)
 *         4. 保持1秒以上
 *         5. 解锁成功
 */
static uint8_t App_process_unlock(void)
{
    switch (thr_state)
    {
    case FREE:
        /* 空闲状态: 等待油门推到最高 */
        if (remote_data.thr >= 900)
        {
            thr_state = MAX;
            max_enter_time = xTaskGetTickCount();
        }
        break;

    case MAX:
        /* 油门最高状态: 检查是否保持1秒 */
        if (remote_data.thr < 900)
        {
            if (xTaskGetTickCount() - max_enter_time >= 1000)
            {
                /* 保持超过1秒: 进入下一状态 */
                thr_state = LEAVE_MAX;
            }
            else
            {
                /* 保持不足1秒: 回到空闲状态 */
                thr_state = FREE;
            }
        }
        break;

    case LEAVE_MAX:
        /* 离开最高状态: 等待油门拉到最低 */
        if (remote_data.thr <= 100)
        {
            thr_state = MIN;
            min_enter_time = xTaskGetTickCount();
        }
        break;

    case MIN:
        /* 油门最低状态: 检查是否保持1秒 */
        if (xTaskGetTickCount() - min_enter_time <= 1000)
        {
            /* 1秒内油门离开最低: 回到空闲状态 */
            if (remote_data.thr > 100)
            {
                thr_state = FREE;
            }
        }
        else
        {
            /* 保持超过1秒: 解锁成功 */
            thr_state = UNLOCK;
        }
        break;

    case UNLOCK:
        /* 解锁成功: 保持状态 */
        break;

    default:
        break;
    }

    return (thr_state == UNLOCK) ? 0 : 1;
}

/* ======================== 飞行状态机 ======================== */

/**
 * @brief 飞行状态机处理
 * @note  状态转换图:
 *         IDLE --(解锁成功)--> NORMAL
 *         NORMAL --(定高指令)--> FIX_HEIGHT
 *         NORMAL --(遥控断连)--> FAIL
 *         FIX_HEIGHT --(取消定高)--> NORMAL
 *         FIX_HEIGHT --(遥控断连)--> FAIL
 *         FAIL --(电机停止)--> IDLE
 */
void App_process_flight_state(void)
{
    switch (flight_state)
    {
    case IDLE:
        /* 空闲状态: 等待解锁 */
        if (App_process_unlock() == 0)
        {
            flight_state = NORMAL;
            thr_state = FREE; /* 重置解锁状态 */
        }
        break;

    case NORMAL:
        /* 正常飞行: 检测定高指令 */
        if (remote_data.fix_height == 1)
        {
            flight_state = FIX_HEIGHT;
            remote_data.fix_height = 0;
            /* 记录当前高度作为定高目标 */
            fix_height = Int_VL53L1X_GetDistance();
        }
        /* 检查遥控器断连 */
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
        }
        break;

    case FIX_HEIGHT:
        /* 定高飞行: 检查取消定高指令 */
        if (remote_data.fix_height == 1)
        {
            flight_state = NORMAL;
            remote_data.fix_height = 0;
        }
        /* 检查遥控器断连 */
        if (remote_state == REMOTE_DISCONNECTED)
        {
            flight_state = FAIL;
        }
        break;

    case FAIL:
        /* 故障状态: 等待电机完全停止 */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        flight_state = IDLE;
        break;

    default:
        break;
    }
}
