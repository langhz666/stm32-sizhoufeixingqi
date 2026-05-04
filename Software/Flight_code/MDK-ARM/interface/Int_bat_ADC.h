/**
 * @file    Int_bat_ADC.h
 * @brief   电池电压检测模块 - ADC采样
 * @author  langhz666
 * @date    2025-09-27
 * @note    通过ADC1读取电池电压，用于低电压保护
 *          电池电压经过分压电阻后接入ADC
 */

#ifndef __INT_BAT_ADC__
#define __INT_BAT_ADC__

#include "adc.h"

/**
 * @brief 初始化电池ADC检测
 * @note  使能ADC采样电路，启动ADC转换
 */
void Int_bat_ADC_Init(void);

/**
 * @brief 读取电池电压
 * @return 电池电压值 (V)
 * @note   电压计算: ADC值 * 3.3V / 4095 * 2 (分压电阻)
 */
float Int_bat_ADC_Read(void);

#endif /* __INT_BAT_ADC__ */
