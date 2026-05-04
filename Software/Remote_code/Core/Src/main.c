/**
 * @file    main.c
 * @brief   P02遥控器主程序入口
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本文件是遥控器固件的主入口点，负责：
 *          - 系统初始化（HAL、时钟、外设）
 *          - SI24R1无线模块初始化
 *          - FreeRTOS任务系统启动
 *
 * 初始化顺序：
 * 1. HAL_Init() - 配置Flash预取、SysTick
 * 2. SystemClock_Config() - 72MHz时钟配置（HSE 8MHz + PLL x9）
 * 3. MX_*_Init() - 外设初始化（GPIO、DMA、USART1、SPI1、ADC1）
 * 4. Int_SI24R1_Init() - 2.4G模块初始化（必须在FreeRTOS之前）
 * 5. App_freeRTOS_start() - 创建任务并启动调度器
 *
 * 时钟树配置：
 * - HSE: 8MHz外部晶振
 * - PLL: HSE x9 = 72MHz
 * - SYSCLK: 72MHz
 * - AHB (HCLK): 72MHz (DIV1)
 * - APB1 (PCLK1): 36MHz (DIV2) - 最大36MHz
 * - APB2 (PCLK2): 72MHz (DIV1)
 * - ADC: 12MHz (PCLK2/6)
 *
 * @note   FreeRTOS启动后，所有代码在任务上下文中执行
 *         main()中的while(1)循环永远不会被执行
 *
 * @note   此文件由STM32CubeMX自动生成，手动修改可能被覆盖
 *         用户代码应放在USER CODE BEGIN/END区域内
 */

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "App_freeRTOS_Task.h"  /* FreeRTOS任务管理 */
#include "Int_SI24R1.h"         /* 2.4G无线模块 */
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

/**
 * @brief  系统时钟配置函数
 *
 * @details 配置PLL和总线时钟：
 *          - HSE: 8MHz外部晶振
 *          - PLL: HSE x9 = 72MHz
 *          - AHB: 72MHz
 *          - APB1: 36MHz
 *          - APB2: 72MHz
 *          - ADC: 12MHz
 */
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  应用程序入口点
 *
 * @details 系统初始化流程：
 *          1. HAL_Init() - 初始化HAL库
 *          2. SystemClock_Config() - 配置系统时钟72MHz
 *          3. MX_GPIO_Init() - 初始化GPIO引脚
 *          4. MX_DMA_Init() - 初始化DMA控制器
 *          5. MX_USART1_UART_Init() - 初始化调试串口
 *          6. MX_SPI1_Init() - 初始化SPI1（用于SI24R1）
 *          7. MX_ADC1_Init() - 初始化ADC1（用于摇杆）
 *          8. Int_SI24R1_Init() - 初始化2.4G模块
 *          9. App_freeRTOS_start() - 启动FreeRTOS
 *
 * @note   此函数永远不会返回
 * @note   FreeRTOS启动后，控制权交给任务调度器
 *
 * @return int（实际上永远不会返回）
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();             /* GPIO引脚初始化 */
    MX_DMA_Init();              /* DMA控制器初始化 */
    MX_USART1_UART_Init();      /* USART1调试串口初始化 */
    MX_SPI1_Init();             /* SPI1初始化（用于SI24R1） */
    MX_ADC1_Init();             /* ADC1初始化（用于摇杆） */

    /* USER CODE BEGIN 2 */

    /* 输出调试信息 */
    debug_printf("Hello remote!\n");

    /* 初始化SI24R1无线模块（必须在FreeRTOS之前） */
    Int_SI24R1_Init();

    /* 启动FreeRTOS - 此后控制权交给内核 */
    App_freeRTOS_start();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /**
         * 此循环永远不会被执行
         * FreeRTOS调度器启动后，控制权交给任务
         */
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief  系统时钟配置
 *
 * @details 配置PLL和总线时钟：
 *
 * 时钟源：
 * - HSE: 8MHz外部晶振（Bypass模式）
 * - HSI: 内部RC振荡器（备用）
 * - PLL: HSE x9 = 72MHz
 *
 * 总线时钟：
 * - SYSCLK: 72MHz（PLL输出）
 * - AHB (HCLK): 72MHz (DIV1)
 * - APB1 (PCLK1): 36MHz (DIV2) - 最大36MHz限制
 * - APB2 (PCLK2): 72MHz (DIV1)
 *
 * ADC时钟：
 * - ADCCLK: 12MHz (PCLK2/6)
 *
 * Flash等待周期：
 * - FLASH_LATENCY_2: 2个等待周期（72MHz需要）
 *
 * @note   如果时钟配置失败，会调用Error_Handler()
 * @note   此函数由STM32CubeMX自动生成
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
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

    /** Initializes the CPU, AHB and APB buses clocks
     */
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

    /* ADC时钟配置：PCLK2/6 = 12MHz */
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
 *
 * @details 当系统初始化失败时调用：
 *          1. 禁用所有中断
 *          2. 进入无限循环
 *          3. 系统必须复位才能恢复
 *
 * @note   用户可在此函数中添加错误指示：
 *          - LED闪烁
 *          - 串口输出错误信息
 *          - 蜂鸣器报警
 *
 * @note   此函数永远不会返回
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  断言失败处理函数
 *
 * @param  file 源文件名
 * @param  line 断言失败的行号
 *
 * @details 当assert_param()宏检查失败时调用：
 *          - 输出文件名和行号
 *          - 用于调试参数错误
 *
 * @note   仅在USE_FULL_ASSERT定义时有效
 * @note   此函数由STM32CubeMX自动生成
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
