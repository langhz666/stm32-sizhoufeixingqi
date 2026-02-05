#include "Int_joystick.h"


uint16_t adc_buff[4] = {0};


/**
 * @brief 初始化ADC遥控  打开ADC
 *
 */
void Int_joystick_init(void)
{
    // 直接使用HAL中的函数  开启ADC
    // 16位数据的地址值  其实是32位   32位数据的地址值也是32位
    // 32单片机 使用的ADC转换为12位精度  范围 0-4095
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff,4);
}

/**
 * @brief 读取ADC数据 保存到结构体地址中
 * 
 * @param joystick 
 */
void Int_joystick_get(Joystick_Struct *joystick)
{
    // DMA不依赖与CPU的计算的  所以读取的数据是实时保存到ADC_BUFF中
    // 顺序是自定义的RAN1234  一定要对齐
    joystick->thr = adc_buff[0];
    joystick->yaw = adc_buff[1];
    joystick->pit = adc_buff[2];
    joystick->rol = adc_buff[3];
}
