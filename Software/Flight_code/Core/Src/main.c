/**
 * @file    main.c
 * @brief   主程序入口 - STM32F103C8T6四旋翼飞控
 * @author  langhz666
 * @date    2025-09-27
 * @note    系统初始化流程:
 *          1. HAL库初始化
 *          2. 系统时钟配置 (72MHz)
 *          3. 外设初始化 (GPIO/UART/TIM/SPI/I2C/ADC)
 *          4. SI24R1无线模块初始化
 *          5. 启动FreeRTOS任务调度
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "App_freeRTOS_Task.h"
#include "Int_SI24R1.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  程序入口
 * @retval int
 * @note   系统初始化后启动FreeRTOS调度器，不会返回
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* 复位所有外设，初始化Flash接口和Systick */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* 配置系统时钟 (72MHz) */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* 初始化所有配置的外设 */
  MX_GPIO_Init();
  MX_USART2_UART_Init();   /* 调试串口 */
  MX_TIM1_Init();           /* 右下电机PWM */
  MX_TIM2_Init();           /* 右上电机PWM */
  MX_TIM3_Init();           /* 左上电机PWM */
  MX_TIM4_Init();           /* 左下电机PWM */
  MX_SPI1_Init();           /* SI24R1无线模块 */
  MX_I2C1_Init();           /* MPU6050 */
  MX_I2C2_Init();           /* VL53L1X */
  MX_ADC1_Init();           /* 电池电压检测 */
  /* USER CODE BEGIN 2 */

  /* 打印启动信息 */
  debug_printf("Hello World!\n");

  /* 初始化SI24R1无线模块 */
  Int_SI24R1_Init();

  /* 启动FreeRTOS任务调度 (后续代码不会执行) */
  App_freeRTOS_start();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 此处代码不会执行 (FreeRTOS调度器已接管) */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief 系统时钟配置
 * @note  时钟源: HSE (8MHz外部晶振)
 *        PLL倍频: 9倍 (8MHz * 9 = 72MHz)
 *        AHB: 72MHz, APB1: 36MHz, APB2: 72MHz
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* 初始化HSE和PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* 初始化总线时钟 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /* 配置ADC时钟 */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  错误处理函数
 * @retval None
 * @note   发生错误时关闭中断，进入死循环
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  断言失败处理函数
 * @param  file: 源文件名
 * @param  line: 错误行号
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* 可以在此添加断言失败的处理逻辑 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
