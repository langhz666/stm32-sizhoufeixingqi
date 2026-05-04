/**
 * @file    App_process_data.c
 * @brief   遥控器数据处理模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责处理遥控器的原始输入数据，包括：
 *          - 摇杆ADC数据采集与校准
 *          - 按键事件处理与微调值调整
 *          - 数据范围映射与限幅
 *
 * 数据处理流程：
 * 1. 读取原始ADC值（0-4095）
 * 2. 映射到控制范围（0-1000）
 * 3. 应用零偏校准
 * 4. 应用按键微调
 * 5. 限幅到有效范围
 *
 * @note    所有数据处理在临界区内进行，防止多任务访问冲突
 */

#include "App_process_data.h"

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 摇杆原始数据结构
 *
 * 存储从ADC读取的原始值，经过处理后用于控制
 */
static Joystick_Struct joystick = {0};

/**
 * @brief 遥控器控制数据（全局共享）
 *
 * 包含处理后的摇杆值和控制标志，
 * 通过extern在其他模块中访问
 */
Remote_Data remote_data = {0};

/* ======================== 微调偏移量 ======================== */

/**
 * @brief 按键微调偏移量
 *
 * 通过方向键调节，每次按下增加/减少10个单位
 * 用于精细调节Pitch和Roll的中位值
 */
static int16_t key_pit_offset = 0;      /**< Pitch微调值（UP增加，DOWN减少） */
static int16_t key_roll_offset = 0;     /**< Roll微调值（RIGHT增加，LEFT减少） */

/**
 * @brief 校准偏移量
 *
 * 通过长按RIGHT_X键触发校准，记录摇杆中位值的偏差
 * 后续采集时减去此偏差，实现零点校准
 */
static int16_t thr_offset = 0;          /**< 油门零偏（通常为0） */
static int16_t yaw_offset = 0;          /**< 偏航零偏 */
static int16_t pit_offset = 0;          /**< 俯仰零偏 */
static int16_t rol_offset = 0;          /**< 横滚零偏 */

/* ======================== 函数实现 ======================== */

/**
 * @brief  校准摇杆中位值
 *
 * @details 采集10次摇杆数据取平均值，计算与期望中位值的偏差：
 *          - THR期望值：0（无油门）
 *          - YAW/PIT/ROL期望值：500（中位）
 *
 *          校准过程：
 *          1. 重置按键微调偏移
 *          2. 连续采集10次摇杆数据
 *          3. 计算每次采集与期望值的差值
 *          4. 累加差值并取平均
 *          5. 将平均值加到校准偏移量上
 *
 * @note   校准时摇杆应保持在中立位置
 * @note   校准结果累加到偏移量，支持多次校准
 * @note   每次采集间隔10ms，总校准时间约100ms
 */
void App_calibrate_joystick(void)
{
    /* 重置按键微调偏移 */
    key_pit_offset = 0;
    key_roll_offset = 0;

    /* 累加器初始化 */
    int16_t thr_sum = 0;
    int16_t yaw_sum = 0;
    int16_t pit_sum = 0;
    int16_t rol_sum = 0;

    /* 采集10次取平均 */
    for (uint8_t i = 0; i < 10; i++)
    {
        /* 读取当前摇杆值 */
        App_process_joystick_data();

        /* 计算与期望值的偏差 */
        thr_sum += joystick.thr - 0;        /* 油门期望值为0 */
        yaw_sum += joystick.yaw - 500;      /* 偏航期望值为500 */
        pit_sum += joystick.pit - 500;      /* 俯仰期望值为500 */
        rol_sum += joystick.rol - 500;      /* 横滚期望值为500 */

        /* 等待10ms再采集下一次 */
        vTaskDelay(10);
    }

    /* 更新校准偏移量（累加平均值） */
    thr_offset += thr_sum / 10;
    yaw_offset += yaw_sum / 10;
    pit_offset += pit_sum / 10;
    rol_offset += rol_sum / 10;
}

/**
 * @brief  处理按键输入事件
 *
 * @details 扫描按键并处理相应事件：
 *          - KEY_UP：Pitch微调增加（+10）
 *          - KEY_DOWN：Pitch微调减少（-10）
 *          - KEY_LEFT：Roll微调减少（-10）
 *          - KEY_RIGHT：Roll微调增加（+10）
 *          - KEY_LEFT_X：设置关机标志
 *          - KEY_RIGHT_X：切换定高模式
 *          - KEY_RIGHT_X_LONG：触发摇杆校准
 *
 * @note   微调值范围建议在±200以内，过大可能导致控制异常
 * @note   关机和定高标志在发送后自动清除
 */
void App_process_key_data(void)
{
    /* 获取当前按键状态 */
    Key_type key = Int_key_get();

    /* 根据按键类型执行相应操作 */
    switch (key)
    {
        case KEY_UP:
            /* Pitch微调增加 - 机头向上 */
            key_pit_offset += 10;
            break;

        case KEY_DOWN:
            /* Pitch微调减少 - 机头向下 */
            key_pit_offset -= 10;
            break;

        case KEY_LEFT:
            /* Roll微调减少 - 向左倾斜 */
            key_roll_offset -= 10;
            break;

        case KEY_RIGHT:
            /* Roll微调增加 - 向右倾斜 */
            key_roll_offset += 10;
            break;

        case KEY_LEFT_X:
            /* 设置关机标志 - 下次发送时传给飞机 */
            remote_data.shutdown = 1;
            break;

        case KEY_RIGHT_X:
            /* 切换定高模式 - 短按触发 */
            remote_data.fix_height = 1;
            break;

        case KEY_RIGHT_X_LONG:
            /* 摇杆校准 - 长按(>1s)触发 */
            App_calibrate_joystick();
            break;

        default:
            /* 无按键按下或未处理的按键 */
            break;
    }
}

/**
 * @brief  处理摇杆ADC数据
 *
 * @details 完整的数据处理流水线：
 *          1. 进入临界区，防止数据被其他任务修改
 *          2. 读取原始ADC值（通过DMA自动更新）
 *          3. 范围映射：0-4095 → 0-1000（取反，适配物理安装方向）
 *          4. 应用校准偏移：减去零偏值
 *          5. 应用按键微调：加上微调值
 *          6. 限幅处理：确保值在0-1000范围内
 *          7. 退出临界区
 *          8. 将处理后的数据复制到remote_data结构
 *
 * @note   临界区保护防止joy_task和key_task同时访问joystick数据
 * @note   取反操作是因为摇杆物理安装方向与期望输出方向相反
 * @note   Com_limit()函数确保数据不会超出有效范围
 */
void App_process_joystick_data(void)
{
    /* ========== 进入临界区 - 防止数据竞争 ========== */
    taskENTER_CRITICAL();

    /* 步骤1：读取原始ADC值 */
    Int_joystick_get(&joystick);

    /* 步骤2：范围映射 ADC(0-4095) → 控制(0-1000) */
    /* 取反操作：物理方向与期望方向相反 */
    joystick.thr = 1000 - (joystick.thr * 1000 / 4095);
    joystick.yaw = 1000 - (joystick.yaw * 1000 / 4095);
    joystick.pit = 1000 - (joystick.pit * 1000 / 4095);
    joystick.rol = 1000 - (joystick.rol * 1000 / 4095);

    /* 步骤3：应用校准偏移 */
    joystick.thr -= thr_offset;
    joystick.yaw -= yaw_offset;
    joystick.pit -= pit_offset;
    joystick.rol -= rol_offset;

    /* 步骤4：应用按键微调 */
    joystick.pit += key_pit_offset;
    joystick.rol += key_roll_offset;

    /* 步骤5：限幅处理 */
    joystick.thr = Com_limit(joystick.thr, 0, 1000);
    joystick.yaw = Com_limit(joystick.yaw, 0, 1000);
    joystick.pit = Com_limit(joystick.pit, 0, 1000);
    joystick.rol = Com_limit(joystick.rol, 0, 1000);

    /* ========== 退出临界区 ========== */
    taskEXIT_CRITICAL();

    /* 步骤6：复制到remote_data结构 */
    remote_data.thr = joystick.thr;
    remote_data.yaw = joystick.yaw;
    remote_data.pit = joystick.pit;
    remote_data.rol = joystick.rol;
}
