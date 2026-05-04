/**
 * @file    Int_joystick.c
 * @brief   摇杆ADC采集模块
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本模块负责通过ADC采集4通道摇杆数据：
 *          - 使用DMA自动传输，减少CPU占用
 *          - 连续转换模式，实时更新数据
 *          - 12位分辨率，范围0-4095
 *
 * ADC通道映射：
 * - CH1 (PA1): THR（油门）
 * - CH6 (PA6): YAW（偏航）
 * - CH2 (PA2): PIT（俯仰）
 * - CH3 (PA3): ROL（横滚）
 *
 * 硬件连接：
 * - 摇杆电位器输出0-3.3V
 * - ADC参考电压3.3V
 * - 12位分辨率：0-4095对应0-3.3V
 *
 * @note   DMA自动更新adc_buff[]，无需CPU干预
 * @note   采集周期由FreeRTOS任务控制（20ms）
 * @note   数据处理在App_process_data模块中完成
 */

#include "Int_joystick.h"

/**
 * @brief ADC DMA缓冲区
 *
 * 存储4通道ADC转换结果，DMA自动更新
 * 通道顺序：[0]=THR, [1]=YAW, [2]=PIT, [3]=ROL
 * 数据格式：12位无符号整数（0-4095）
 */
static uint16_t adc_buff[4] = {0};

/**
 * @brief  初始化摇杆ADC（DMA模式）
 *
 * @details 启动ADC1的DMA连续转换：
 *          - 4通道扫描模式
 *          - 连续转换使能
 *          - DMA循环模式
 *          - 数据右对齐
 *
 * @note   初始化后ADC开始自动转换
 * @note   adc_buff[]由DMA自动更新，无需手动触发
 * @note   调用Int_joystick_get()获取最新值
 */
void Int_joystick_init(void)
{
    /**
     * HAL_ADC_Start_DMA参数说明：
     * - &hadc1: ADC1句柄
     * - (uint32_t *)adc_buff: DMA目标地址（32位对齐）
     * - 4: 转换通道数
     *
     * 注意：adc_buff是uint16_t类型，需要强制转换为uint32_t*
     * 因为HAL库要求32位对齐的地址
     */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 4);
}

/**
 * @brief  获取摇杆ADC值
 *
 * @param  joystick 指向Joystick_Struct的指针，用于存储结果
 *
 * @details 从DMA缓冲区读取最新ADC值：
 *          - adc_buff[0] -> THR（油门）
 *          - adc_buff[1] -> YAW（偏航）
 *          - adc_buff[2] -> PIT（俯仰）
 *          - adc_buff[3] -> ROL（横滚）
 *
 * @note   DMA在后台自动更新adc_buff[]
 * @note   读取操作是原子的（16位读取）
 * @note   数据范围：0-4095（12位ADC）
 *
 * @warning 在多任务环境下，建议在临界区内调用此函数
 *          防止读取到不一致的数据
 */
void Int_joystick_get(Joystick_Struct *joystick)
{
    /**
     * DMA自动更新说明：
     * - DMA配置为循环模式
     * - ADC每次转换完成后自动传输到adc_buff[]
     * - CPU无需干预，直接读取即可
     *
     * 通道顺序由CubeMX配置决定：
     * - Rank 1: CH1 -> adc_buff[0] (THR)
     * - Rank 2: CH6 -> adc_buff[1] (YAW)
     * - Rank 3: CH2 -> adc_buff[2] (PIT)
     * - Rank 4: CH3 -> adc_buff[3] (ROL)
     */
    joystick->thr = adc_buff[0];    /* 油门：PA1 -> ADC1_IN1 */
    joystick->yaw = adc_buff[1];    /* 偏航：PA6 -> ADC1_IN6 */
    joystick->pit = adc_buff[2];    /* 俯仰：PA2 -> ADC1_IN2 */
    joystick->rol = adc_buff[3];    /* 横滚：PA3 -> ADC1_IN3 */
}
