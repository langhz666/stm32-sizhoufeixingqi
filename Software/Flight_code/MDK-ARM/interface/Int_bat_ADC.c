#include "Int_bat_ADC.h"

/**
 * @brief 启动ADC采样
 * 
 */
void Int_bat_ADC_Init(void)
{
    // 1. 打开使能引脚
    HAL_GPIO_WritePin(BAT_ADC_EN_GPIO_Port, BAT_ADC_EN_Pin, GPIO_PIN_RESET);

    // 2. 完成ADC初始化
    HAL_ADC_Start(&hadc1);
}

/**
 * @brief 直接读取电池电压
 * 
 * @return float 
 */
float Int_bat_ADC_Read(void)
{
    // 14位精度ADC的数值
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);

    // 2. 电压值 = ADC * 3.3 / 4095
    float voltage = (adc_value * 3.3 / 4095) * 2;
    return voltage;
}
