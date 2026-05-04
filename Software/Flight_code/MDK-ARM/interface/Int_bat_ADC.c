/**
 * @file    Int_bat_ADC.c
 * @brief   电池电压检测实现
 * @author  langhz666
 * @date    2025-09-27
 * @note    12位ADC，参考电压3.3V，分压电阻比1:2
 */

#include "Int_bat_ADC.h"

/**
 * @brief 初始化电池ADC检测
 * @note  1. 使能ADC采样电路 (拉低EN引脚)
 *        2. 启动ADC转换
 */
void Int_bat_ADC_Init(void)
{
    /* 1. 使能ADC采样电路 */
    HAL_GPIO_WritePin(BAT_ADC_EN_GPIO_Port, BAT_ADC_EN_Pin, GPIO_PIN_RESET);

    /* 2. 启动ADC */
    HAL_ADC_Start(&hadc1);
}

/**
 * @brief 读取电池电压
 * @return 电池电压值 (V)
 * @note   电压 = ADC值 * 3.3V / 4095 * 2
 *         乘以2是因为使用了1:1分压电阻
 */
float Int_bat_ADC_Read(void)
{
    /* 读取12位ADC值 */
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);

    /* 计算实际电压 (考虑分压电阻) */
    float voltage = (adc_value * 3.3 / 4095) * 2;
    return voltage;
}
