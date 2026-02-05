#include "Int_VL53L1X.h"
uint16_t dev = 0x52;
/**
 * @brief 初始化激光测距仪  完成寄存器的配置
 *
 */
void Int_VL53L1X_Init(void)
{
    // 1. 重置芯片  => XSHUT引脚拉低再拉高
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_RESET);
    vTaskDelay(2);
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_SET);
    vTaskDelay(2);

    uint8_t byteData = 0;
    uint16_t wordData = 0;
    uint8_t sensorState = 0;
    /* Those basic I2C read functions can be used to check your own I2C functions */
    VL53L1_RdByte(dev, 0x010F, &byteData);
    printf("VL53L1X Model_ID: %X\n", byteData);
    VL53L1_RdByte(dev, 0x0110, &byteData);
    printf("VL53L1X Module_Type: %X\n", byteData);
    VL53L1_RdWord(dev, 0x010F, &wordData);
    printf("VL53L1X: %X\n", wordData);
    while (sensorState == 0)
    {
        VL53L1X_BootState(dev, &sensorState);
        vTaskDelay(2);
    }
    printf("Chip booted\n");

    /* This function must to be called to initialize the sensor with the default setting  */
    VL53L1X_SensorInit(dev);
    /* Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
    VL53L1X_SetDistanceMode(dev, 2);          /* 1=short, 2=long */
    VL53L1X_SetTimingBudgetInMs(dev, 20);     /* in ms possible values [20, 50, 100, 200, 500] */
    VL53L1X_SetInterMeasurementInMs(dev, 20); /* in ms, IM must be > = TB */
                                              //  status = VL53L1X_SetOffset(dev,20); /* offset compensation in mm */
                                              //  status = VL53L1X_SetROI(dev, 16, 16); /* minimum ROI 4,4 */
                                              //	status = VL53L1X_CalibrateOffset(dev, 140, &offset); /* may take few second to perform the offset cal*/
                                              //	status = VL53L1X_CalibrateXtalk(dev, 1000, &xtalk); /* may take few second to perform the xtalk cal */
    printf("VL53L1X Ultra Lite Driver Example running ...\n");
    VL53L1X_StartRanging(dev); /* This function has to be called to enable the ranging */
}

/**
 * @brief 读取激光测距仪的距离值
 *
 */
uint16_t Int_VL53L1X_GetDistance(void)
{
    uint16_t Distance = 0;
    VL53L1X_GetDistance(dev, &Distance);
    return Distance;
}
